#ifndef CHANNELTABLISTMODEL_H
#define CHANNELTABLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>
#include <QDebug>

class LinkHandlerModel;
class ChannelModel;
class Extractor;
class TabBarLookup;
class TabBarTabInfo;

class ChannelTabListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  enum ChannelTabListRoles {
    ModelDataRole = Qt::UserRole + 1,
    TitleRole,
    IconRole,
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
  void addAboutTab(Extractor* extractor, QQmlComponent* tab);

signals:

private:
  QObject* createTabObject(QQmlComponent* component);
  TabBarTabInfo* createTabInfo(QQmlComponent* tab, QString const& icon);

private:
  QHash<int, QByteArray> m_roles;
  QMap<QString, TabBarLookup*> m_tabBarLookup;
  QList<TabBarTabInfo*> m_tabInfo;
  TabBarTabInfo* m_aboutTab;
};

#endif // CHANNELTABLISTMODEL_H
