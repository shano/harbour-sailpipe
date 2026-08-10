#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDBusContext>

class QQmlEngine;
class QJSEngine;
class DownloadContext;
class TransferEngineClient;
class DBusAdapter;
class YtDlpDownloadContext;

class DownloadManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString page READ page WRITE setPage NOTIFY pageChanged)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(DownloadStatus downloadStatus READ downloadStatus NOTIFY downloadStatusChanged)
  Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

public:
  enum DownloadStatus {
    None,
    Running,
    Done,
    Cancelled,
    Error,
  };
  Q_ENUM(DownloadStatus)

  explicit DownloadManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static DownloadManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  QString page() const;
  void setPage(QString page);
  QString name() const;
  void setName(QString name);
  DownloadStatus downloadStatus() const;
  float progress() const;

public slots:
  void downloadFile(QString const url);
  void downloadFileWithName(QString const url, QString const leafname);
  void cancel();

  // DBus interface
  void cancelDownload(int transferId);
  void restartDownload(int transferId);

private slots:
  void onReadyRead();
  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onFinished(QNetworkReply* reply);
  void onError(QNetworkReply::NetworkError code);
  void setProgress(float progress);
  void onFinalise();

signals:
  void pageChanged();
  void nameChanged();
  void downloadStatusChanged();
  void progressChanged();

private:
  void setDownloadStatus(DownloadStatus downloadStatus);
  void destroyContext(DownloadContext* context);
  QString constructFilename(QString const& name, QString const& extension);

private:
  static DownloadManager* m_instance;
  QNetworkAccessManager* m_manager;
  QMap<QString, DownloadContext*> m_running;
  QMap<int, DownloadContext*> m_transferIds;
  QString m_page;
  QString m_name;
  DownloadStatus m_downloadStatus;
  float m_progress;
  TransferEngineClient* m_transferClient;
  DBusAdapter* m_dbusAdapter;
  bool m_dbusRegistered;
  YtDlpDownloadContext* m_ytDlpContext;
};

#endif // DOWNLOADMANAGER_H
