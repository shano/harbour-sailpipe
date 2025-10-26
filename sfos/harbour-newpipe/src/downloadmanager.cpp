#include <QNetworkRequest>
#include <QNetworkReply>
#include <transferengineclient.h>

#include "downloadcontext.h"
#include "downloadmanager.h"

#define MAX_DATA_CHUNK (1024 * 64)

DownloadManager* DownloadManager::m_instance = nullptr;
static const QMap<QString, QString> mimetypes = {
  {"video/mp4", "mp4"},
};

DownloadManager::DownloadManager(QObject *parent)
  : QObject(parent)
  , m_manager(new QNetworkAccessManager(this))
  , m_running()
  , m_page()
  , m_downloadStatus(None)
  , m_progress(0.0)
  , m_transferClient(new TransferEngineClient(this))
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

QString DownloadManager::page() const
{
  return m_page;
}

void DownloadManager::setPage(QString page)
{
  if (m_page != page) {
    DownloadContext* context = m_running.value(m_page);
    if (context) {
      disconnect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    }

    m_page = page;

    if (m_running.contains(m_page)) {
      DownloadContext* context = m_running.value(m_page);
      connect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
      setProgress(context->progress());
      setDownloadStatus(context->downloadStatus());
    }
    else {
      setDownloadStatus(DownloadStatus::None);
    }
    emit pageChanged();
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
  QNetworkRequest request;

  qDebug() << "Download URL: " << url;
  request.setUrl(QUrl(url));
  request.setMaximumRedirectsAllowed(5);
  QNetworkReply* reply = m_manager->get(request);

  connect(reply, &QNetworkReply::readyRead, this, &DownloadManager::onReadyRead);
  connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
  connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadManager::onError);

  DownloadContext* context = new DownloadContext(m_page, m_transferClient, this);
  reply->setProperty("context", QVariant::fromValue(context));

  if (m_running.contains(m_page)) {
    delete m_running.value(m_page);
  }
  m_running.insert(m_page, context);

  connect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
  connect(context, &DownloadContext::finalise, this, &DownloadManager::onFinalise);
  setProgress(0.0);
  setDownloadStatus(DownloadStatus::Running);
}

void DownloadManager::downloadFileWithName(QString const url, QString const leafname)
{
}

void DownloadManager::onReadyRead()
{
  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  DownloadContext* context;
  bool more;

  if (reply != nullptr ) {
    context = reply->property("context").value<DownloadContext*>();

    if (!context->ready()) {
      QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
      QString extension = mimetypes.value(contentType, QString::fromLatin1("dat"));
      QString filename = QString::fromLatin1("/home/defaultuser/Downloads/%1.%2").arg(QString::fromLatin1("video"), extension);
      qlonglong length = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
      context->open(filename, contentType, length);
    }

    more = true;
    while (more) {
      QByteArray data = reply->read(MAX_DATA_CHUNK);
      more = (data.length() > 0);
      if (more) {
        context->write(data);
      }
    }
  }
}

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  if (reply != nullptr ) {
    DownloadContext* context = reply->property("context").value<DownloadContext*>();
    float progress = (float)bytesReceived / (float)bytesTotal;
    context->setProgress(progress);
    if (m_page == context->page()) {
      setProgress(progress);
    }
  }
}

void DownloadManager::onFinished(QNetworkReply* reply)
{
  if (reply != nullptr ) {
    DownloadContext* context = reply->property("context").value<DownloadContext*>();
    qDebug() << "onFinished: written: " << context->written();
    QList<QNetworkReply::RawHeaderPair> const headers = reply->rawHeaderPairs();
    for (QNetworkReply::RawHeaderPair const header : headers) {
      qDebug() << "onFinished: header: " << header.first << " = " << header.second;
    }

    context->done();
    reply->deleteLater();
  }
}

void DownloadManager::onError(QNetworkReply::NetworkError code)
{
  qDebug() << "onError: error:" << code;
}

void DownloadManager::onFinalise()
{
  DownloadContext* context = dynamic_cast<DownloadContext*>(sender());
  if (context) {
    if (m_page == context->page()) {
      setDownloadStatus(None);
    }
    disconnect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    disconnect(context, &DownloadContext::finalise, this, &DownloadManager::onFinalise);
    m_running.remove(context->page());
    delete context;
  }
}
