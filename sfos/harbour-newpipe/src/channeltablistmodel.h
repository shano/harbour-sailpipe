#ifndef CHANNELTABLISTMODEL_H
#define CHANNELTABLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>
#include <QDebug>

class LinkHandlerModel;
class ChannelModel;
class Extractor;

class ChannelTabListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  enum ChannelTabListRoles {
    ModelDataRole = Qt::UserRole + 1,
    TitleRole,
    ModelRole,
    NoitemsRole,
  };

  explicit ChannelTabListModel(QObject *parent = nullptr);

public slots:
  QHash<int, QByteArray> roleNames() const;

  int rowCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

  int count();

  void generateModel(Extractor* extractor, LinkHandlerModel* linkHandlerModel);
  ChannelModel* getModel(int row) const;

signals:

private:
  QHash<int, QByteArray> m_roles;
  QList<QObject*> m_components;
  QList<ChannelModel*> m_models;
};

#endif // CHANNELTABLISTMODEL_H
