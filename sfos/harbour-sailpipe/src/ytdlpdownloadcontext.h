#ifndef YTDLPDOWNLOADCONTEXT_H
#define YTDLPDOWNLOADCONTEXT_H

#include <QObject>
#include <QProcess>

#include "downloadmanager.h"

class YtDlpDownloadContext : public QObject
{
  Q_OBJECT

  Q_PROPERTY(DownloadManager::DownloadStatus downloadStatus READ downloadStatus NOTIFY downloadStatusChanged)

public:
  explicit YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, QObject *parent = nullptr);
  ~YtDlpDownloadContext();

  void start();
  void cancel();

  QString page() const;
  DownloadManager::DownloadStatus downloadStatus() const;
  float progress() const;

signals:
  void downloadStatusChanged(DownloadManager::DownloadStatus downloadStatus);
  void progressChanged(float progress);
  void finalise();

private slots:
  void onReadyReadStandardOutput();
  void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  void setDownloadStatus(DownloadManager::DownloadStatus downloadStatus);
  void setProgress(float progress);

private:
  QString m_page;
  QString m_sourceUrl;
  QString m_targetPath;
  QProcess* m_process;
  DownloadManager::DownloadStatus m_downloadStatus;
  float m_progress;
};

#endif // YTDLPDOWNLOADCONTEXT_H
