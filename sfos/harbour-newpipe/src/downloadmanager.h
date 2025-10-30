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

class DownloadManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString page READ page WRITE setPage NOTIFY pageChanged)
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
  void downloadStatusChanged();
  void progressChanged();

private:
  void setDownloadStatus(DownloadStatus downloadStatus);
  void destroyContext(DownloadContext* context);

private:
  static DownloadManager* m_instance;
  QNetworkAccessManager* m_manager;
  QMap<QString, DownloadContext*> m_running;
  QMap<int, DownloadContext*> m_transferIds;
  QString m_page;
  DownloadStatus m_downloadStatus;
  float m_progress;
  TransferEngineClient* m_transferClient;
  DBusAdapter* m_dbusAdapter;
  bool m_dbusRegistered;
};

#endif // DOWNLOADMANAGER_H
