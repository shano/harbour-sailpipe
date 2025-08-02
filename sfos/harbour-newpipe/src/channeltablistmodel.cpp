#include <sailfishapp.h>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

#include "listlinkhandler.h"
#include "linkhandlermodel.h"
#include "channelmodel.h"
#include "titlebarlookup.h"

#include "channeltablistmodel.h"

ChannelTabListModel::ChannelTabListModel(QObject *parent)
  : QAbstractListModel(parent)
{
  m_roles[TitleRole] = "titlehidden";
  m_roles[IconRole] = "icon";
  m_roles[ModelDataRole] = "modelData";
  m_roles[ModelRole] = "channelmodel";
  m_roles[NoitemsRole] = "noitems";

  m_titleBarLookup.insert("about", new TitleBarLookup("About", "image://newpipe/icon-tab-about", this));
  m_titleBarLookup.insert("videos", new TitleBarLookup("Videos", "image://newpipe/icon-tab-video-full", this));
  m_titleBarLookup.insert("shorts", new TitleBarLookup("Shorts", "image://newpipe/icon-tab-video-short", this));
  m_titleBarLookup.insert("livestreams", new TitleBarLookup("Live", "image://newpipe/icon-tab-video-live", this));
  m_titleBarLookup.insert("playlists", new TitleBarLookup("Playlists", "image://newpipe/icon-tab-playlists", this));
}

QHash<int, QByteArray> ChannelTabListModel::roleNames() const
{
  return m_roles;
}

int ChannelTabListModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent)
  return m_components.count();
}

QVariant ChannelTabListModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_components.count())) {
    QObject* component = m_components[index.row()];
    ChannelModel* model = m_models[index.row()];
    switch (role) {
      case ModelDataRole: {
        result = QVariant::fromValue(component);
      }
      break;
      case TitleRole: {
        TitleBarLookup*lookup = m_titleBarLookup.value(model->linkHandler()->contentFilters()[0]);
        if (lookup) {
          result = QVariant(lookup->title());
        }
        else {
          QString title = model->linkHandler()->contentFilters()[0];
          title[0] = title[0].toUpper();
          result = QVariant(title);
        }
      }
      break;
      case IconRole: {
        TitleBarLookup*lookup = m_titleBarLookup.value(model->linkHandler()->contentFilters()[0]);
        if (lookup) {
          result = QVariant(lookup->icon());
        }
        else {
          result = QString("");
        }
      }
      break;
      case ModelRole: {
        result = QVariant::fromValue(model);
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
  return m_components.count();
}

ChannelModel* ChannelTabListModel::getModel(int row) const
{
  return m_models[row];
}

void ChannelTabListModel::generateModel(Extractor* extractor, LinkHandlerModel* linkHandlerModel)
{
  beginResetModel();
  int count = linkHandlerModel->count();
  m_components.clear();
  m_models.clear();

  QQmlEngine* engine = qmlEngine(this);
  for (int tab = 0; tab < count; tab++) {
    ListLinkHandler* linkHandler = linkHandlerModel->getLinkHandler(tab);

    QQmlComponent* tabComponent = new QQmlComponent(engine, SailfishApp::pathTo("qml/components/Tab.qml"));
    QQmlComponent* component = new QQmlComponent(engine, SailfishApp::pathTo("qml/components/ChannelListView.qml"));
    ChannelModel* channelmodel = new ChannelModel(this);

    QQmlContext* context = new QQmlContext(qmlContext(this));
    QObject* object = tabComponent->create(context);
    object->setProperty("body", QVariant::fromValue(component));

    m_components.append(object);
    m_models.append(channelmodel);
    channelmodel->setLinkHandler(linkHandler);
    channelmodel->search(extractor);
  }

  endResetModel();
}

