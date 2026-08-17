#include <QDBusConnection>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <transferengineclient.h>

#include "dbusadapter.h"
#include "downloadmanager.h"
#include "ytdlpdownloadcontext.h"

#define SHORTNAME_CHARS (23)
#define SHORTNAME_REPLACE "[_\\-,.]"
#define SHORTNAME_REMOVE "[^[:alnum:]\\s]"

DownloadManager* DownloadManager::m_instance = nullptr;

DownloadManager::DownloadManager(QObject *parent)
  : QObject(parent)
  , m_page()
  , m_downloadStatus(None)
  , m_progress(0.0)
  , m_transferClient(new TransferEngineClient(this))
  , m_dbusRegistered(false)
  , m_ytDlpContext(nullptr)
{
  m_dbusAdapter = new DBusAdapter(this);
  QDBusConnection connection = QDBusConnection::sessionBus();
  m_dbusRegistered = connection.registerObject(QString::fromLatin1("/"), this);
  if (m_dbusRegistered) {
    m_dbusRegistered = connection.registerService(QString::fromLatin1("io.github.shano.harbour-sailreel"));
  }
}

void DownloadManager::instantiate(QObject* parent) {
  if (m_instance == nullptr) {
    m_instance = new DownloadManager(parent);
  }
}

DownloadManager& DownloadManager::getInstance() {
  return *m_instance;
}

QObject* DownloadManager::provider(QQmlEngine* engine, QJSEngine* scriptEngine) {
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

QString DownloadManager::page() const
{
  return m_page;
}

void DownloadManager::setPage(QString page)
{
  if (m_page != page) {
    if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
      disconnect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
      disconnect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
    }

    m_page = page;

    if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
      connect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
      connect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
      setProgress(m_ytDlpContext->progress());
      setDownloadStatus(m_ytDlpContext->downloadStatus());
    }
    else {
      setDownloadStatus(DownloadStatus::None);
    }
    emit pageChanged();
  }
}

QString DownloadManager::name() const
{
  return m_name;
}

void DownloadManager::setName(QString name)
{
  if (m_name != name) {
    m_name = name;
    emit nameChanged();
  }
}

DownloadManager::DownloadStatus DownloadManager::downloadStatus() const
{
  return m_downloadStatus;
}

float DownloadManager::progress() const
{
  return m_progress;
}

void DownloadManager::setDownloadStatus(DownloadStatus downloadStatus)
{
  if (m_downloadStatus != downloadStatus) {
    m_downloadStatus = downloadStatus;
    switch (m_downloadStatus) {
      case DownloadStatus::None:
        setProgress(0.0);
        break;
      case DownloadStatus::Done:
        setProgress(1.0);
        break;
      default:
        // Do nothing
        break;
    }

    emit downloadStatusChanged();
  }
}

void DownloadManager::setProgress(float progress)
{
  if (m_progress != progress) {
    m_progress = progress;
    emit progressChanged();
  }
}

void DownloadManager::downloadFile(QString const url)
{
  QString extension = QStringLiteral("mp4");
  QString targetPath = constructFilename(m_name, extension);

  if (m_ytDlpContext) {
    // Leave `finalise` connected so the superseded context still reaches
    // onFinalise() and gets deleteLater()'d once its process exits; only
    // the two signals that would directly corrupt the displayed status/
    // progress of the new download are disconnected.
    disconnect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    disconnect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
  }

  m_ytDlpContext = new YtDlpDownloadContext(m_page, m_page, targetPath, m_transferClient, m_dbusRegistered, this);
  connect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
  connect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
  connect(m_ytDlpContext, &YtDlpDownloadContext::finalise, this, &DownloadManager::onFinalise);

  setProgress(0.0);
  m_ytDlpContext->start();
}

void DownloadManager::downloadFileWithName(QString const url, QString const leafname)
{
}

void DownloadManager::onFinalise()
{
  YtDlpDownloadContext* ytDlpContext = dynamic_cast<YtDlpDownloadContext*>(sender());
  if (ytDlpContext) {
    // Only touch the displayed status if this is still the tracked context —
    // a superseded context (see downloadFile()) stays connected to finalise()
    // purely for cleanup and must not affect what the current download shows,
    // even if it happens to share the same page.
    if (m_ytDlpContext == ytDlpContext) {
      if (m_page == ytDlpContext->page()) {
        setDownloadStatus(None);
      }
      m_ytDlpContext = nullptr;
    }
    ytDlpContext->deleteLater();
  }
}

void DownloadManager::cancel()
{
  qDebug() << "DownloadManager cancel: " << m_page;
  if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
    m_ytDlpContext->cancel();
  }
  else {
    qDebug() << "Page to cancel not found: " << m_page;
  }
}

void DownloadManager::cancelDownload(int transferId)
{
  qDebug() << "DownloadManager cancelDownload: " << transferId;
  if (m_ytDlpContext && (m_ytDlpContext->transferId() == transferId)) {
    m_ytDlpContext->cancel();
  }
  else {
    qDebug() << "Transfer ID to cancel not found: " << transferId;
  }
}

void DownloadManager::restartDownload(int transferId)
{
  qDebug() << "DownloadManager restartDownload: " << transferId;
}

QString DownloadManager::constructFilename(QString const& name, QString const& extension)
{
  bool exists;
  QStandardPaths::StandardLocation location = (extension == QStringLiteral("mp4"))
    ? QStandardPaths::MoviesLocation
    : QStandardPaths::DownloadLocation;
  QString directory = QStandardPaths::writableLocation(location);

  QDir check(directory);
  exists = check.exists();

  if (!exists) {
    exists = check.mkpath(directory);
  }

  if (!exists) {
    qDebug() << "Download directory doesn't exist and couldn't be created";
  }

  qint64 epoch = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000;
  QString date = QString("%1").arg(epoch, 8, 16, QChar('0'));

  QString shortname = name;
  shortname = shortname.replace(QRegExp(QString::fromLatin1(SHORTNAME_REPLACE)), QChar(' '));
  shortname = shortname.remove(QRegExp(QString::fromLatin1(SHORTNAME_REMOVE)));
  shortname = shortname.toLower();
  shortname = shortname.simplified();
  shortname.truncate(SHORTNAME_CHARS);
  if (shortname.isEmpty()) {
    shortname = QString::fromLatin1("media");
  }
  shortname = shortname.replace(QRegExp(QString::fromLatin1("\\s+")), QChar('_'));
  QString result = QString::fromLatin1("%1/%2-%3.%4").arg(directory, shortname, date, extension);

  qDebug() << "Filename inputs: " << name << ", " << extension;
  qDebug() << "Filename output: " << result;

  return result;
}
