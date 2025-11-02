#include "downloadmanager.h"

#include "dbusadapter.h"

DBusAdapter::DBusAdapter(DownloadManager* downloadManager)
  : QDBusAbstractAdaptor(downloadManager)
  , m_downloadManager(downloadManager)
{

}

void DBusAdapter::cancelDownload(int transferId)
{
  m_downloadManager->cancelDownload(transferId);
}

void DBusAdapter::restartDownload(int transferId)
{
  m_downloadManager->restartDownload(transferId);
}
