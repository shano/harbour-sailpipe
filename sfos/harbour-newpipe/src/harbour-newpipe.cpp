#include <QtQuick>

#include <sailfishapp.h>

#include "searchmodel.h"
#include "channelmodel.h"
#include "commentmodel.h"
#include "extractor.h"
#include "mediainfo.h"
#include "utils.h"
#include "imageprovider.h"
#include "pageref.h"
#include "channelinfo.h"
#include "channeltabinfo.h"
#include "listlinkhandler.h"
#include "linkhandlermodel.h"
#include "channeltablistmodel.h"
#include "playlistmodel.h"
#include "filtermodel.h"
#include "downloadmanager.h"

int main(int argc, char *argv[])
{
  int result;

  QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
  QCoreApplication::setOrganizationName("uk.co.flypig");
  QCoreApplication::setApplicationName("harbour-newpipe");
  QScopedPointer<QQuickView> view(SailfishApp::createView());
  QQmlContext* ctxt = view->rootContext();
  QQmlEngine* engine = view->engine();
  QScopedPointer<Extractor> extractor(new Extractor(app.data()));
  Utils::instantiate();
  DownloadManager::instantiate();

  qmlRegisterType<Extractor>("harbour.newpipe.extractor", 1, 0, "Extractor");
  qmlRegisterType<SearchModel>("harbour.newpipe.extractor", 1, 0, "SearchModel");
  qmlRegisterType<ChannelModel>("harbour.newpipe.extractor", 1, 0, "ChannelModel");
  qmlRegisterType<CommentModel>("harbour.newpipe.extractor", 1, 0, "CommentModel");
  qmlRegisterType<PlaylistModel>("harbour.newpipe.extractor", 1, 0, "PlaylistModel");
  qmlRegisterType<MediaInfo>("harbour.newpipe.extractor", 1, 0, "MediaInfo");
  qmlRegisterType<PageRef>("harbour.newpipe.extractor", 1, 0, "PageRef");
  qmlRegisterType<SearchItem>("harbour.newpipe.extractor", 1, 0, "SearchItem");
  qmlRegisterType<ChannelInfo>("harbour.newpipe.extractor", 1, 0, "ChannelInfo");
  qmlRegisterType<ChannelTabInfo>("harbour.newpipe.extractor", 1, 0, "ChannelTabInfo");
  qmlRegisterType<ListLinkHandler>("harbour.newpipe.extractor", 1, 0, "ListLinkHandler");
  qmlRegisterType<LinkHandlerModel>("harbour.newpipe.extractor", 1, 0, "LinkHandlerModel");
  qmlRegisterType<ChannelTabListModel>("harbour.newpipe.extractor", 1, 0, "ChannelTabListModel");
  qmlRegisterType<FilterModel>("harbour.newpipe.extractor", 1, 0, "FilterModel");
  qmlRegisterSingletonType<Utils>("harbour.newpipe.extractor", 1, 0, "Utils", Utils::provider);
  qmlRegisterSingletonType<DownloadManager>("harbour.newpipe.extractor", 1, 0, "DownloadManager", DownloadManager::provider);

  engine->addImageProvider(QLatin1String("newpipe"), new ImageProvider());

  ctxt->setContextProperty("extractor", extractor.data());

  view->setSource(SailfishApp::pathTo("qml/harbour-newpipe.qml"));
  view->show();
  result = app->exec();

  return result;
}
