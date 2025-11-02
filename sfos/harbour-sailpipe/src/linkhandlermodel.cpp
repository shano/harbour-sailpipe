#include "channelinfo.h"
#include "listlinkhandler.h"

#include "linkhandlermodel.h"

LinkHandlerModel::LinkHandlerModel(QObject *parent)
  : QAbstractListModel(parent)
{
  m_roles[LinkHandlerRole] = "linkHanlder";
}

QHash<int, QByteArray> LinkHandlerModel::roleNames() const
{
  return m_roles;
}

int LinkHandlerModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent)
  return m_linkHandlers.count();
}

void LinkHandlerModel::setModel(ChannelInfo *channelInfo)
{
  beginResetModel();
  m_linkHandlers = channelInfo->tabs();
  endResetModel();
}

QVariant LinkHandlerModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_linkHandlers.count())) {
    ListLinkHandler* linkHandler = m_linkHandlers[index.row()];
    switch (role) {
      case LinkHandlerRole:
        result = QVariant::fromValue(linkHandler);
        break;
    }
  }

  return result;
}

int LinkHandlerModel::count()
{
  return m_linkHandlers.count();
}

ListLinkHandler* LinkHandlerModel::getLinkHandler(quint32 index) const
{
  return m_linkHandlers.at(index);
}
