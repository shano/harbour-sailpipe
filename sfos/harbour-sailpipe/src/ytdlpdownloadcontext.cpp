#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QUrl>
#include <transferengineclient.h>

#include "ytdlpmanager.h"
#include "ytdlpdownloadcontext.h"

#define DONE_TIMEOUT (60000)
#define YTDLP_MIMETYPE "video/mp4"

YtDlpDownloadContext::YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, TransferEngineClient* transferClient, bool dbusRegistered, QObject *parent)
  : QObject(parent)
  , m_page(page)
  , m_sourceUrl(sourceUrl)
  , m_targetPath(targetPath)
  , m_process(new QProcess(this))
  , m_timer()
  , m_downloadStatus(DownloadManager::None)
  , m_progress(0.0)
  , m_finalised(false)
  , m_transferClient(transferClient)
  , m_transferId(0)
  , m_dbusRegistered(dbusRegistered)
{
  m_timer.setInterval(DONE_TIMEOUT);
  m_timer.setSingleShot(true);
  m_timer.stop();
  connect(&m_timer, &QTimer::timeout, this, &YtDlpDownloadContext::onTimeout);

  connect(m_process, &QProcess::readyReadStandardOutput, this, &YtDlpDownloadContext::onReadyReadStandardOutput);
  connect(m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &YtDlpDownloadContext::onFinished);
  connect(m_process, &QProcess::errorOccurred, this, &YtDlpDownloadContext::onErrorOccurred);
}

YtDlpDownloadContext::~YtDlpDownloadContext()
{
  m_timer.stop();
}

void YtDlpDownloadContext::start()
{
  setDownloadStatus(DownloadManager::Running);

  QUrl localFile = QUrl::fromLocalFile(m_targetPath);
  QString const cancelDownload = m_dbusRegistered ? QString::fromLatin1("cancelDownload") : QString::fromLatin1("");
  CallbackInterface callback(QString::fromLatin1("uk.co.flypig.sailpipe"), QString::fromLatin1("/"), QString::fromLatin1("uk.co.flypig.sailpipe"), cancelDownload, QString::fromLatin1(""));
  // yt-dlp writes the file itself, so the final size isn't known up front.
  qlonglong length = 0;
  //% "SailPipe download"
  m_transferId = m_transferClient->createDownloadEvent(qtTrId("sailpipe_transfer_engine-sailpipe_download"), QUrl(), QUrl(QString::fromLatin1("image://theme/harbour-sailpipe")), localFile, QString::fromLatin1(YTDLP_MIMETYPE), length, callback);
  m_transferClient->startTransfer(m_transferId);
  qDebug() << "Started yt-dlp download with transfer ID: " << m_transferId;

  QStringList args;
  args << QStringLiteral("-f") << QStringLiteral("best[ext=mp4]/best")
       << QStringLiteral("-o") << m_targetPath
       << QStringLiteral("--newline")
       << QStringLiteral("--progress-template") << QStringLiteral("download:PROGRESS %(progress._percent_str)s")
       << m_sourceUrl;

  m_process->start(YtDlpManager::binaryPath(), args);
}

void YtDlpDownloadContext::cancel()
{
  if (m_finalised) {
    return;
  }

  // Set the terminal status before terminating the process: waitForFinished()
  // below delivers QProcess::finished synchronously, and onFinished() only
  // leaves the status alone if it already reads Cancelled.
  setDownloadStatus(DownloadManager::Cancelled);

  if (m_process->state() != QProcess::NotRunning) {
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
      m_process->kill();
      m_process->waitForFinished(1000);
    }
  }

  if (!m_finalised) {
    finaliseDownload();
  }
}

QString YtDlpDownloadContext::page() const
{
  return m_page;
}

DownloadManager::DownloadStatus YtDlpDownloadContext::downloadStatus() const
{
  return m_downloadStatus;
}

float YtDlpDownloadContext::progress() const
{
  return m_progress;
}

int YtDlpDownloadContext::transferId() const
{
  return m_transferId;
}

void YtDlpDownloadContext::setDownloadStatus(DownloadManager::DownloadStatus downloadStatus)
{
  if (m_downloadStatus != downloadStatus) {
    m_downloadStatus = downloadStatus;
    emit downloadStatusChanged(m_downloadStatus);
  }
}

void YtDlpDownloadContext::setProgress(float progress)
{
  if (m_progress != progress) {
    m_progress = progress;
    if (m_transferId != 0) {
      m_transferClient->updateTransferProgress(m_transferId, m_progress);
    }
    emit progressChanged(m_progress);
  }
}

void YtDlpDownloadContext::finaliseDownload()
{
  m_finalised = true;

  TransferEngineClient::Status status = TransferEngineClient::TransferFinished;
  if (m_downloadStatus == DownloadManager::Cancelled) {
    status = TransferEngineClient::TransferCanceled;
  }
  else if (m_downloadStatus != DownloadManager::Done) {
    status = TransferEngineClient::TransferInterrupted;
  }

  if (m_downloadStatus != DownloadManager::Done) {
    removePartialFiles();
  }

  // Dwell on the terminal status long enough for the "done" indicator to be
  // visible, matching DownloadContext's behaviour, before emitting finalise().
  m_timer.start();

  if (m_transferId != 0) {
    m_transferClient->finishTransfer(m_transferId, status, QString());
  }
}

void YtDlpDownloadContext::removePartialFiles()
{
  QFile::remove(m_targetPath);
  QFile::remove(m_targetPath + QStringLiteral(".part"));
}

void YtDlpDownloadContext::onReadyReadStandardOutput()
{
  static QRegularExpression const percentPattern(QStringLiteral("PROGRESS\\s+([0-9.]+)%"));

  QByteArray data = m_process->readAllStandardOutput();
  QStringList lines = QString::fromUtf8(data).split(QChar('\n'), QString::SkipEmptyParts);
  for (QString const& line : lines) {
    QRegularExpressionMatch match = percentPattern.match(line);
    if (match.hasMatch()) {
      setProgress(match.captured(1).toFloat() / 100.0f);
    }
  }
}

void YtDlpDownloadContext::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (m_finalised) {
    return;
  }
  m_finalised = true;

  if (m_downloadStatus != DownloadManager::Cancelled) {
    if ((exitStatus == QProcess::NormalExit) && (exitCode == 0)) {
      setProgress(1.0);
      setDownloadStatus(DownloadManager::Done);
    }
    else {
      setDownloadStatus(DownloadManager::Error);
    }
  }

  finaliseDownload();
}

void YtDlpDownloadContext::onErrorOccurred(QProcess::ProcessError error)
{
  Q_UNUSED(error)

  if (m_finalised) {
    return;
  }
  m_finalised = true;

  if (m_downloadStatus != DownloadManager::Cancelled) {
    setDownloadStatus(DownloadManager::Error);
  }

  finaliseDownload();
}

void YtDlpDownloadContext::onTimeout()
{
  setDownloadStatus(DownloadManager::None);
  emit finalise();
}
