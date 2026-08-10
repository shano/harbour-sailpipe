#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSysInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QNetworkRequest>
#include <QUrlQuery>

#include "ytdlpmanager.h"

#define GITHUB_LATEST_RELEASE_URL "https://api.github.com/repos/yt-dlp/yt-dlp/releases/latest"

YtDlpManager* YtDlpManager::m_instance = nullptr;

YtDlpManager::YtDlpManager(QObject *parent)
  : QObject(parent)
  , m_manager(new QNetworkAccessManager(this))
  , m_activeReply(nullptr)
  , m_status(NotInstalled)
  , m_installedVersion()
  , m_latestVersion()
  , m_progress(0.0)
{
  refreshInstalledVersion();
}

void YtDlpManager::instantiate(QObject* parent)
{
  if (m_instance == nullptr) {
    m_instance = new YtDlpManager(parent);
  }
}

YtDlpManager& YtDlpManager::getInstance()
{
  return *m_instance;
}

QObject* YtDlpManager::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

QString YtDlpManager::binaryPath()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return QString("%1/yt-dlp/yt-dlp").arg(dataDir);
}

QString YtDlpManager::releaseAssetName()
{
  QString arch = QSysInfo::currentCpuArchitecture();
  if (arch == QLatin1String("arm64")) {
    return QStringLiteral("yt-dlp_linux_aarch64");
  }
  // armv7hl devices report "arm" from QSysInfo; only the zipped asset
  // exists upstream for this architecture. Extraction is handled by the
  // caller (install()) when the asset name ends in .zip.
  return QStringLiteral("yt-dlp_linux_armv7l.zip");
}

YtDlpManager::Status YtDlpManager::status() const
{
  return m_status;
}

QString YtDlpManager::installedVersion() const
{
  return m_installedVersion;
}

QString YtDlpManager::latestVersion() const
{
  return m_latestVersion;
}

float YtDlpManager::progress() const
{
  return m_progress;
}

void YtDlpManager::setStatus(Status status)
{
  if (m_status != status) {
    m_status = status;
    emit statusChanged();
  }
}

void YtDlpManager::setInstalledVersion(QString const& version)
{
  if (m_installedVersion != version) {
    m_installedVersion = version;
    emit installedVersionChanged();
  }
}

void YtDlpManager::setLatestVersion(QString const& version)
{
  if (m_latestVersion != version) {
    m_latestVersion = version;
    emit latestVersionChanged();
  }
}

void YtDlpManager::setProgress(float progress)
{
  if (m_progress != progress) {
    m_progress = progress;
    emit progressChanged();
  }
}

void YtDlpManager::refreshInstalledVersion()
{
  QFileInfo info(binaryPath());
  if (!info.exists() || !info.isExecutable()) {
    setInstalledVersion(QString());
    setStatus(NotInstalled);
    return;
  }

  QProcess process;
  process.start(binaryPath(), QStringList() << QStringLiteral("--version"));
  if (process.waitForFinished(5000) && (process.exitCode() == 0)) {
    QString version = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    setInstalledVersion(version);
    setStatus(Installed);
  }
  else {
    setInstalledVersion(QString());
    setStatus(Error);
  }
}

void YtDlpManager::checkForUpdate()
{
  setStatus(CheckingForUpdate);

  QNetworkRequest request{QUrl(QStringLiteral(GITHUB_LATEST_RELEASE_URL))};
  request.setRawHeader("Accept", "application/vnd.github+json");
  request.setRawHeader("User-Agent", "harbour-sailpipe");

  m_activeReply = m_manager->get(request);
  connect(m_activeReply, &QNetworkReply::finished, this, &YtDlpManager::onLatestReleaseReply);
}

void YtDlpManager::onLatestReleaseReply()
{
  QNetworkReply* reply = m_activeReply;
  m_activeReply = nullptr;

  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
    reply->deleteLater();
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
  QString tag = doc.object()["tag_name"].toString();
  setLatestVersion(tag);
  setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);

  reply->deleteLater();
}

void YtDlpManager::install()
{
  QString assetName = releaseAssetName();
  QString downloadUrl = QString("https://github.com/yt-dlp/yt-dlp/releases/latest/download/%1").arg(assetName);
  startDownload(downloadUrl);
}

void YtDlpManager::update()
{
  install();
}

void YtDlpManager::startDownload(QString const& downloadUrl)
{
  setStatus(Downloading);
  setProgress(0.0);

  QNetworkRequest request{QUrl(downloadUrl)};
  request.setMaximumRedirectsAllowed(10);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  m_activeReply = m_manager->get(request);
  connect(m_activeReply, &QNetworkReply::downloadProgress, this, &YtDlpManager::onAssetDownloadProgress);
  connect(m_activeReply, &QNetworkReply::finished, this, &YtDlpManager::onAssetDownloadFinished);
}

void YtDlpManager::onAssetDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if (bytesTotal > 0) {
    setProgress(static_cast<float>(bytesReceived) / static_cast<float>(bytesTotal));
  }
}

void YtDlpManager::onAssetDownloadFinished()
{
  QNetworkReply* reply = m_activeReply;
  m_activeReply = nullptr;

  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
    reply->deleteLater();
    return;
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  QString path = binaryPath();
  QDir().mkpath(QFileInfo(path).absolutePath());

  QString assetName = releaseAssetName();
  bool ok = false;
  if (assetName.endsWith(QStringLiteral(".zip"))) {
    // armv7l asset ships zipped. Extraction requires the `unzip`
    // command-line tool to be present on-device.
    QString tmpZip = QString("%1.zip").arg(path);
    QFile zipFile(tmpZip);
    if (zipFile.open(QIODevice::WriteOnly)) {
      zipFile.write(data);
      zipFile.close();

      QProcess unzip;
      unzip.setWorkingDirectory(QFileInfo(path).absolutePath());
      unzip.start(QStringLiteral("unzip"), QStringList() << QStringLiteral("-o") << tmpZip);
      ok = unzip.waitForFinished(30000) && (unzip.exitCode() == 0);
      QFile::remove(tmpZip);

      if (ok) {
        // The zip contains a single file; find it and rename to binaryPath().
        QDir dir(QFileInfo(path).absolutePath());
        QStringList entries = dir.entryList(QDir::Files);
        for (QString const& entry : entries) {
          if (entry != QFileInfo(path).fileName() && !entry.endsWith(QStringLiteral(".zip"))) {
            QFile::remove(path);
            ok = QFile::rename(dir.filePath(entry), path);
            break;
          }
        }
      }
    }
  }
  else {
    QFile outFile(path);
    ok = outFile.open(QIODevice::WriteOnly);
    if (ok) {
      outFile.write(data);
      outFile.close();
    }
  }

  if (ok) {
    QFile::setPermissions(path, QFile::permissions(path)
      | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    refreshInstalledVersion();
  }
  else {
    emit errorOccurred(QStringLiteral("Failed to install yt-dlp binary"));
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
  }
}
