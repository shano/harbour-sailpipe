#include <QNetworkRequest>
#include <QNetworkReply>

#include "downloadmanager.h"

#define MAX_DATA_CHUNK (1024 * 64)

DownloadManager* DownloadManager::m_instance = nullptr;

DownloadManager::DownloadManager(QObject *parent)
  : QObject(parent)
  , m_manager(new QNetworkAccessManager(this))
{
  connect(m_manager, &QNetworkAccessManager::finished, this, &DownloadManager::onFinished);
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

void DownloadManager::downloadFile(QString const url)
{
  QNetworkRequest request;

  qDebug() << "String: " << url;
  request.setUrl(QUrl(url));
  QNetworkReply* reply = m_manager->get(request);

  connect(reply, &QNetworkReply::readyRead, this, &DownloadManager::onReadyRead);
  connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
  connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadManager::onError);
}

void DownloadManager::downloadFileWithName(QString const url, QString const leafname)
{
}

void DownloadManager::onReadyRead()
{
  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  if (reply != nullptr ) {
    quint64 length;
    bool more;
    quint64 read;

    length = 0;
    more = true;
    while (more) {
      QByteArray data = reply->read(MAX_DATA_CHUNK);
      read = data.length();
      if (read > 0) {
        length += read;
      }
      else {
        more = false;
      }
    }
    qDebug() << "onReadyRead: read length: " << length;
  }
}

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  if (reply != nullptr ) {
    qDebug() << "onReadyRead: progress: " << bytesReceived << " out of " << bytesTotal;
  }
}

void DownloadManager::onFinished(QNetworkReply* reply)
{
  if (reply != nullptr ) {
    qDebug() << "onFinished";
    reply->deleteLater();
  }
}

void DownloadManager::onError(QNetworkReply::NetworkError code)
{
  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  if (reply != nullptr ) {
    qDebug() << "onError: error:" << code;
  }
}
