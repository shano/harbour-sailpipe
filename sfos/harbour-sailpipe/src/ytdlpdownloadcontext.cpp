#include <QRegularExpression>

#include "ytdlpmanager.h"
#include "ytdlpdownloadcontext.h"

YtDlpDownloadContext::YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, QObject *parent)
  : QObject(parent)
  , m_page(page)
  , m_sourceUrl(sourceUrl)
  , m_targetPath(targetPath)
  , m_process(new QProcess(this))
  , m_downloadStatus(DownloadManager::None)
  , m_progress(0.0)
  , m_finalised(false)
{
  connect(m_process, &QProcess::readyReadStandardOutput, this, &YtDlpDownloadContext::onReadyReadStandardOutput);
  connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &YtDlpDownloadContext::onFinished);
  connect(m_process, &QProcess::errorOccurred, this, &YtDlpDownloadContext::onErrorOccurred);
}

YtDlpDownloadContext::~YtDlpDownloadContext()
{
}

void YtDlpDownloadContext::start()
{
  setDownloadStatus(DownloadManager::Running);

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
  if (m_process->state() != QProcess::NotRunning) {
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
      m_process->kill();
    }
  }
  setDownloadStatus(DownloadManager::Cancelled);
  emit finalise();
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
    emit progressChanged(m_progress);
  }
}

void YtDlpDownloadContext::onReadyReadStandardOutput()
{
  static QRegularExpression const percentPattern(QStringLiteral("PROGRESS\\s+([0-9.]+)%"));

  QByteArray data = m_process->readAllStandardOutput();
  QStringList lines = QString::fromUtf8(data).split(QChar('\n'), Qt::SkipEmptyParts);
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

  if ((exitStatus == QProcess::NormalExit) && (exitCode == 0)) {
    setProgress(1.0);
    setDownloadStatus(DownloadManager::Done);
  }
  else if (m_downloadStatus != DownloadManager::Cancelled) {
    setDownloadStatus(DownloadManager::Error);
  }
  emit finalise();
}

void YtDlpDownloadContext::onErrorOccurred(QProcess::ProcessError error)
{
  Q_UNUSED(error)

  if (m_finalised) {
    return;
  }
  m_finalised = true;

  setDownloadStatus(DownloadManager::Error);
  emit finalise();
}
