#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDBusConnection>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <transferengineclient.h>

#include "dbusadapter.h"
#include "downloadcontext.h"
#include "downloadmanager.h"
#include "ytdlpdownloadcontext.h"

#define MAX_DATA_CHUNK (1024 * 64)
#define SHORTNAME_CHARS (23)
#define SHORTNAME_REPLACE "[_\\-,.]"
#define SHORTNAME_REMOVE "[^[:alnum:]\\s]"

DownloadManager* DownloadManager::m_instance = nullptr;
static const QMap<QString, QString> mimetypes = {
  {"video/mp4", "mp4"},
  {"audio/mpegurl", "m3u8"},
  {"application/x-bittorrent", "torrent"},
  {"audio/mpeg", "mp3"},
  {"text/html", "html"},
  {"text/plain", "txt"},
};

DownloadManager::DownloadManager(QObject *parent)
  : QObject(parent)
  , m_manager(new QNetworkAccessManager(this))
  , m_running()
  , m_transferIds()
  , m_page()
  , m_downloadStatus(None)
  , m_progress(0.0)
  , m_transferClient(new TransferEngineClient(this))
  , m_dbusRegistered(false)
  , m_ytDlpContext(nullptr)
{
  connect(m_manager, &QNetworkAccessManager::finished, this, &DownloadManager::onFinished);

  m_dbusAdapter = new DBusAdapter(this);
  QDBusConnection connection = QDBusConnection::sessionBus();
  m_dbusRegistered = connection.registerObject(QString::fromLatin1("/"), this);
  if (m_dbusRegistered) {
    m_dbusRegistered = connection.registerService(QString::fromLatin1("io.github.shano.sailreel"));
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
    DownloadContext* context = m_running.value(m_page);
    if (context) {
      disconnect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    }

    if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
      disconnect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
      disconnect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
    }

    m_page = page;

    if (m_running.contains(m_page)) {
      DownloadContext* context = m_running.value(m_page);
      connect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
      setProgress(context->progress());
      setDownloadStatus(context->downloadStatus());
    }
    else if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
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
  static QRegularExpression const youtubeHost(QStringLiteral("(^|\\.)youtube\\.com$|(^|\\.)youtu\\.be$"));

  QUrl pageUrl(m_page);
  bool isYouTube = youtubeHost.match(pageUrl.host()).hasMatch();

  if (isYouTube) {
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
    return;
  }

  QNetworkRequest request;

  qDebug() << "Download URL: " << url;
  request.setUrl(QUrl(url));
  request.setMaximumRedirectsAllowed(50);
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, QVariant(false));

  QNetworkReply* reply = m_manager->get(request);

  connect(reply, &QNetworkReply::readyRead, this, &DownloadManager::onReadyRead);
  connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
  connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadManager::onError);

  DownloadContext* context = new DownloadContext(m_page, reply, m_transferClient, m_dbusRegistered, this);
  reply->setProperty("context", QVariant::fromValue(context));

  if (m_running.contains(m_page)) {
    destroyContext(m_running.value(m_page));
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
      QString filename = constructFilename(m_name, extension);
      qlonglong length = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
      context->open(filename, contentType, length);
      m_transferIds.insert(context->transferId(), context);
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
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    DownloadContext* context = reply->property("context").value<DownloadContext*>();
    qDebug() << "onFinished: written: " << context->written();
    qDebug() << "onFinished: status: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QList<QNetworkReply::RawHeaderPair> const headers = reply->rawHeaderPairs();
    for (QNetworkReply::RawHeaderPair const header : headers) {
      qDebug() << "onFinished: header: " << header.first << " = " << header.second;
    }
    context->done();

    if ((status >= 300) && (status < 400)) {
      QString url = reply->header(QNetworkRequest::LocationHeader).toString();
      if (!url.isEmpty()) {
        downloadFile(url);
      }
    }
    reply->deleteLater();
  }
}

void DownloadManager::onError(QNetworkReply::NetworkError code)
{
  qDebug() << "onError: error:" << code;

  QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
  if (reply != nullptr ) {
    DownloadContext* context = reply->property("context").value<DownloadContext*>();
    switch (code) {
      case QNetworkReply::NetworkError::OperationCanceledError:
        context->cancel();
        break;
      case QNetworkReply::NoError:
        // Do nothing
        break;
      default:
        context->error();
        break;
    }
  }
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
    destroyContext(context);
    return;
  }

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

void DownloadManager::destroyContext(DownloadContext* context)
{
  if (context) {
    if (m_transferIds.contains(context->transferId())) {
      m_transferIds.remove(context->transferId());
    }
    if (m_running.contains(context->page())) {
      m_running.remove(context->page());
    }
    delete context;
  }
}

void DownloadManager::cancel()
{
  qDebug() << "DownloadManager cancel: " << m_page;
  if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
    m_ytDlpContext->cancel();
  }
  else if (m_running.contains(m_page)) {
    DownloadContext* context = m_running.value(m_page);
    QNetworkReply* reply = context->reply();
    reply->abort();
  }
  else {
    qDebug() << "Page to cancel not found: " << m_page;
  }
}

void DownloadManager::cancelDownload(int transferId)
{
  qDebug() << "DownloadManager cancelDownload: " << transferId;
  if (m_transferIds.contains(transferId)) {
    DownloadContext* context = m_transferIds.value(transferId);
    QNetworkReply* reply = context->reply();
    reply->abort();
  }
  else if (m_ytDlpContext && (m_ytDlpContext->transferId() == transferId)) {
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
  QString directory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

  QDir check(directory);
  exists = check.exists();

  if (!exists) {
    exists = check.mkpath(directory);
  }

  if (!exists) {
    qDebug() << "Downloads directory doesn't exist and couldn't be created";
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
