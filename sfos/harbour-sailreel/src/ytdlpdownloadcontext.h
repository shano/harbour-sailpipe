#ifndef YTDLPDOWNLOADCONTEXT_H
#define YTDLPDOWNLOADCONTEXT_H

#include <QObject>
#include <QProcess>
#include <QTimer>

#include "downloadmanager.h"

class TransferEngineClient;

class YtDlpDownloadContext : public QObject
{
  Q_OBJECT

  Q_PROPERTY(DownloadManager::DownloadStatus downloadStatus READ downloadStatus NOTIFY downloadStatusChanged)

public:
  explicit YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, TransferEngineClient* transferClient, bool dbusRegistered, QObject *parent = nullptr);
  ~YtDlpDownloadContext();

  void start();
  void cancel();

  QString page() const;
  DownloadManager::DownloadStatus downloadStatus() const;
  float progress() const;
  int transferId() const;

signals:
  void downloadStatusChanged(DownloadManager::DownloadStatus downloadStatus);
  void progressChanged(float progress);
  void finalise();

private slots:
  void onReadyReadStandardOutput();
  void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onErrorOccurred(QProcess::ProcessError error);
  void onTimeout();

private:
  void setDownloadStatus(DownloadManager::DownloadStatus downloadStatus);
  void setProgress(float progress);
  void finaliseDownload();
  void removePartialFiles();

private:
  QString m_page;
  QString m_sourceUrl;
  QString m_targetPath;
  QProcess* m_process;
  QTimer m_timer;
  DownloadManager::DownloadStatus m_downloadStatus;
  float m_progress;
  bool m_finalised;
  TransferEngineClient* m_transferClient;
  int m_transferId;
  bool m_dbusRegistered;
};

#endif // YTDLPDOWNLOADCONTEXT_H
