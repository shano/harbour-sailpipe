#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QQmlEngine;
class QJSEngine;

class DownloadManager : public QObject
{
  Q_OBJECT
public:
  explicit DownloadManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static DownloadManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

public slots:
  void downloadFile(QString const url);
  void downloadFileWithName(QString const url, QString const leafname);

private slots:
  void onReadyRead();
  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onFinished(QNetworkReply* reply);
  void onError(QNetworkReply::NetworkError code);

signals:

private:
  static DownloadManager* m_instance;
  QNetworkAccessManager* m_manager;
};

#endif // DOWNLOADMANAGER_H
