#include "downloadcontext.h"

#define DONE_TIMEOUT (60000)

DownloadContext::DownloadContext(QString const& page, QObject *parent)
  : QObject(parent)
  , m_file()
  , m_page(page)
  , m_written(0)
  , m_progress(0.0)
  , m_timer()
  , m_downloadStatus(DownloadManager::Running)
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
  }
}

void DownloadContext::open(QString const& filename)
{
  m_file.setFileName(filename);
  m_file.open(QIODevice::WriteOnly);
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
  if (m_file.isOpen()) {
    m_file.close();
  }
  setDownloadStatus(DownloadManager::Done);
  m_timer.start();
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

void DownloadContext::onTimeout()
{
  setDownloadStatus(DownloadManager::None);
  emit finalise();
}
