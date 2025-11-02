#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H

#include <QDBusAbstractAdaptor>
#include <QObject>

class DownloadManager;

class DBusAdapter : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "uk.co.flypig.sailpipe")

public:
  explicit DBusAdapter(DownloadManager* DownloadManager);

public slots:
  void cancelDownload(int transferId);
  void restartDownload(int transferId);

private:
  DownloadManager* m_downloadManager;
};

#endif // DBUSADAPTER_H
