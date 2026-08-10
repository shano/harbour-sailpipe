#ifndef YTDLPMANAGER_H
#define YTDLPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QQmlEngine;
class QJSEngine;

class YtDlpManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY(Status status READ status NOTIFY statusChanged)
  Q_PROPERTY(QString installedVersion READ installedVersion NOTIFY installedVersionChanged)
  Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestVersionChanged)
  Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

public:
  enum Status {
    NotInstalled,
    Installed,
    CheckingForUpdate,
    Downloading,
    Error,
  };
  Q_ENUM(Status)

  explicit YtDlpManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static YtDlpManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  static QString binaryPath();
  static QString releaseAssetName();

  Status status() const;
  QString installedVersion() const;
  QString latestVersion() const;
  float progress() const;

public slots:
  void refreshInstalledVersion();
  void checkForUpdate();
  void install();
  void update();

signals:
  void statusChanged();
  void installedVersionChanged();
  void latestVersionChanged();
  void progressChanged();
  void errorOccurred(QString message);

private slots:
  void onLatestReleaseReply();
  void onAssetDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onAssetDownloadFinished();

private:
  void setStatus(Status status);
  void setInstalledVersion(QString const& version);
  void setLatestVersion(QString const& version);
  void setProgress(float progress);
  void startDownload(QString const& downloadUrl);

private:
  static YtDlpManager* m_instance;
  QNetworkAccessManager* m_manager;
  QNetworkReply* m_activeReply;
  Status m_status;
  QString m_installedVersion;
  QString m_latestVersion;
  float m_progress;
};

#endif // YTDLPMANAGER_H
