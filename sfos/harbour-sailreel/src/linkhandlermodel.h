#ifndef LINKHANDLERMODEL_H
#define LINKHANDLERMODEL_H

#include <QAbstractListModel>

class ChannelInfo;
class ListLinkHandler;

class LinkHandlerModel : public QAbstractListModel
{
  Q_OBJECT
public:
  enum LinkHandlerRoles {
    LinkHandlerRole = Qt::UserRole + 1,
  };

  explicit LinkHandlerModel(QObject* parent = nullptr);

 public slots:
  QHash<int, QByteArray> roleNames() const;

  int rowCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

  void setModel(ChannelInfo *channelInfo);
  ListLinkHandler* getLinkHandler(quint32 index) const;
  int count();

signals:

private:
  QHash<int, QByteArray> m_roles;
  QList<ListLinkHandler*> m_linkHandlers;
};

#endif // LINKHANDLERMODEL_H
