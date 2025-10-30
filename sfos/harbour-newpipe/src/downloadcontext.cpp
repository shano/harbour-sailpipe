#include <transferengineclient.h>

#include "downloadcontext.h"

#define DONE_TIMEOUT (60000)

DownloadContext::DownloadContext(QString const& page, QNetworkReply* reply, TransferEngineClient* transferClient, QObject *parent)
  : QObject(parent)
  , m_file()
  , m_page(page)
  , m_written(0)
  , m_progress(0.0)
  , m_timer()
  , m_downloadStatus(DownloadManager::Running)
  , m_transferClient(transferClient)
  , m_transferId(0)
  , m_reply(reply)
{
  m_timer.setInterval(DONE_TIMEOUT);
  m_timer.setSingleShot(true);
  m_timer.stop();
  connect(&m_timer, &QTimer::timeout, this, &DownloadContext::onTimeout);
}

DownloadContext::~DownloadContext()
{
  if (m_file.isOpen()) {
    m_file.close();

    if (m_downloadStatus != DownloadManager::Done) {
      m_file.remove();
    }
  }
  m_timer.stop();
}

void DownloadContext::open(QString const& filename, QString const& mimetype, qlonglong length)
{
  m_file.setFileName(filename);
  m_file.open(QIODevice::WriteOnly);

  CallbackInterface callback("uk.co.flypig.newpipe", "/", "uk.co.flypig.newpipe", "cancelDownload", "");
  m_transferId = m_transferClient->createDownloadEvent("NewPipe download",  QUrl(), QUrl("image://theme/harbour-newpipe"), filename, mimetype, length, callback);
  m_transferClient->startTransfer(m_transferId);
  qDebug() << "Started download with transfer ID: " << m_transferId;
}

bool DownloadContext::ready() const
{
  return m_file.isOpen();
}

qint64 DownloadContext::write(QByteArray const& bytes)
{
  qint64 written = 0;

  if (ready()) {
    written = m_file.write(bytes);
    m_written += written;
  }

  return written;
}

QString DownloadContext::page() const
{
  return m_page;
}

void DownloadContext::setProgress(double progress)
{
  m_progress = progress;

  m_transferClient->updateTransferProgress(m_transferId, m_progress);
}

double DownloadContext::progress() const
{
  return m_progress;
}

quint64 DownloadContext::written() const
{
  return m_written;
}

void DownloadContext::done()
{
  TransferEngineClient::Status status;

  if (m_file.isOpen()) {
    m_file.close();

    if (m_downloadStatus != DownloadManager::Done) {
      m_file.remove();
    }
  }

  m_reply = nullptr;
  switch (m_downloadStatus) {
    case DownloadManager::Cancelled:
      status = TransferEngineClient::TransferCanceled;
      break;
    case DownloadManager::Error:
      status = TransferEngineClient::TransferInterrupted;
      break;
    default:
      setDownloadStatus(DownloadManager::Done);
      status = TransferEngineClient::TransferFinished;
      break;
  }

  m_timer.start();

  m_transferClient->finishTransfer(m_transferId, status, QString());
}

DownloadManager::DownloadStatus DownloadContext::downloadStatus() const
{
  return m_downloadStatus;
}

void DownloadContext::setDownloadStatus(DownloadManager::DownloadStatus downloadStatus)
{
  if (m_downloadStatus != downloadStatus) {
    m_downloadStatus = downloadStatus;

    emit downloadStatusChanged(m_downloadStatus);
  }
}

void DownloadContext::cancel()
{
  setDownloadStatus(DownloadManager::Cancelled);
}

void DownloadContext::error()
{
  setDownloadStatus(DownloadManager::Error);
}

void DownloadContext::onTimeout()
{
  setDownloadStatus(DownloadManager::None);
  emit finalise();
}

int DownloadContext::transferId() const
{
  return m_transferId;
}

QNetworkReply* DownloadContext::reply() const
{
  return m_reply;
}
