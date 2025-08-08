#include <sailfishapp.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

#include "listlinkhandler.h"
#include "linkhandlermodel.h"
#include "channelmodel.h"
#include "tabbarlookup.h"
#include "tabbartabinfo.h"

#include "channeltablistmodel.h"

ChannelTabListModel::ChannelTabListModel(QObject *parent)
  : QAbstractListModel(parent)
{
  m_roles[TitleRole] = "titlehidden";
  m_roles[IconRole] = "icon";
  m_roles[ModelDataRole] = "modelData";
  m_roles[ModelRole] = "channelmodel";
  m_roles[NoitemsRole] = "noitems";

  m_tabBarLookup.insert("about", new TabBarLookup("About", "image://newpipe/icon-tab-about", this));
  m_tabBarLookup.insert("videos", new TabBarLookup("Videos", "image://newpipe/icon-tab-video-full", this));
  m_tabBarLookup.insert("shorts", new TabBarLookup("Shorts", "image://newpipe/icon-tab-video-short", this));
  m_tabBarLookup.insert("livestreams", new TabBarLookup("Live", "image://newpipe/icon-tab-video-live", this));
  m_tabBarLookup.insert("playlists", new TabBarLookup("Playlists", "image://newpipe/icon-tab-playlists", this));
  m_tabBarLookup.insert("albums", new TabBarLookup("Albums", "image://newpipe/icon-tab-albums", this));
}

QHash<int, QByteArray> ChannelTabListModel::roleNames() const
{
  return m_roles;
}

int ChannelTabListModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent)
  return m_tabInfo.count();
}

QVariant ChannelTabListModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_tabInfo.count())) {
    TabBarTabInfo* tabInfo = m_tabInfo[index.row()];
    switch (role) {
      case ModelDataRole: {
        result = QVariant::fromValue(tabInfo->component());
      }
      break;
      case TitleRole: {
        result = QVariant(tabInfo->title());
      }
      break;
      case IconRole: {
        result = QVariant(tabInfo->icon());
      }
      break;
      case ModelRole: {
        result = QVariant::fromValue(tabInfo->model());
      }
      break;
      case NoitemsRole: {
        result = QVariant(QString("No items %1").arg(index.row()));
      }
      break;
    }
  }

  return result;
}

int ChannelTabListModel::count()
{
  return m_tabInfo.count();
}

ChannelModel* ChannelTabListModel::getModel(int row) const
{
  return m_tabInfo[row]->model();
}

QObject* ChannelTabListModel::createTabObject(QQmlComponent* component)
{
  QQmlEngine* engine = qmlEngine(this);
  QQmlComponent* tabComponent = new QQmlComponent(engine, SailfishApp::pathTo("qml/components/Tab.qml"), this);

  QQmlContext* context = new QQmlContext(qmlContext(this));
  QObject* object = tabComponent->create(context);
  object->setProperty("body", QVariant::fromValue(component));

  return object;
}

void ChannelTabListModel::addAboutTab(Extractor* extractor, QQmlComponent* aboutTab)
{
  m_aboutTab = createTabInfo(aboutTab, "image://newpipe/icon-tab-about");
}

void ChannelTabListModel::generateModel(Extractor* extractor, LinkHandlerModel* linkHandlerModel)
{
  beginResetModel();
  int count = linkHandlerModel->count();
  m_tabInfo.clear();

  QQmlEngine* engine = qmlEngine(this);
  for (int tab = 0; tab < count; tab++) {
    TabBarTabInfo* tabInfo = new TabBarTabInfo(this);
    ListLinkHandler* linkHandler = linkHandlerModel->getLinkHandler(tab);
    QQmlComponent* component = new QQmlComponent(engine, SailfishApp::pathTo("qml/components/ChannelListView.qml"));
    QObject* object = createTabObject(component);
    ChannelModel* channelmodel = new ChannelModel(this);

    tabInfo->setComponent(object);
    tabInfo->setModel(channelmodel);

    TabBarLookup* lookup = m_tabBarLookup.value(linkHandler->contentFilters()[0]);
    if (lookup) {
      tabInfo->setTitle(lookup->title());
      tabInfo->setIcon(lookup->icon());
    }
    m_tabInfo.append(tabInfo);

    channelmodel->setLinkHandler(linkHandler);
    channelmodel->search(extractor);
  }

  if (m_aboutTab) {
    m_tabInfo.append(m_aboutTab);
  }

  endResetModel();
}

TabBarTabInfo* ChannelTabListModel::createTabInfo(QQmlComponent* tab, QString const& icon)
{
  TabBarTabInfo* tabInfo = new TabBarTabInfo(this);
  QObject* object = createTabObject(tab);

  tabInfo->setIcon(icon);
  tabInfo->setComponent(object);

  return tabInfo;
}
