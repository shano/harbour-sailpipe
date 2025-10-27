#ifndef DOWNLOADCONTEXT_H
#define DOWNLOADCONTEXT_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTimer>

#include "downloadmanager.h"

class TransferEngineClient;
class QNetworkReply;

class DownloadContext : public QObject
{
  Q_OBJECT

  Q_PROPERTY(DownloadManager::DownloadStatus downloadStatus READ downloadStatus NOTIFY downloadStatusChanged)

public:
  DownloadContext(QString const& page, QNetworkReply* reply, TransferEngineClient* transferClient, QObject *parent = nullptr);
  ~DownloadContext();

  qint64 write(QByteArray const& bytes);

  void open(QString const& filename, QString const& mimetype, qlonglong length);
  QString page() const;
  bool ready() const;
  void done();

  void setProgress(double progress);
  double progress() const;
  quint64 written() const;
  DownloadManager::DownloadStatus downloadStatus() const;
  int transferId() const;
  QNetworkReply* reply() const;

signals:
  void downloadStatusChanged(DownloadManager::DownloadStatus downloadStatus);
  void finalise();

private slots:
  void onTimeout();

private:
  void setDownloadStatus(DownloadManager::DownloadStatus downloadStatus);

private:
  QFile m_file;
  QString m_page;
  quint64 m_written;
  double m_progress;
  QTimer m_timer;
  DownloadManager::DownloadStatus m_downloadStatus;
  TransferEngineClient* m_transferClient;
  int m_transferId;
  QNetworkReply* m_reply;
};

#endif // DOWNLOADCONTEXT_H
