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
#include "mediajunction.h"
#include "ytdlpmanager.h"

int main(int argc, char *argv[])
{
  int result;

  QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
  QCoreApplication::setOrganizationName("io.github.shano");
  QCoreApplication::setApplicationName("harbour-sailreel");
  QScopedPointer<QQuickView> view(SailfishApp::createView());
  QQmlContext* ctxt = view->rootContext();
  QQmlEngine* engine = view->engine();
  QScopedPointer<Extractor> extractor(new Extractor(app.data()));
  Utils::instantiate();
  DownloadManager::instantiate();
  MediaJunction::instantiate();
  YtDlpManager::instantiate();

  qmlRegisterType<Extractor>("harbour.sailpipe.extractor", 1, 0, "Extractor");
  qmlRegisterType<SearchModel>("harbour.sailpipe.extractor", 1, 0, "SearchModel");
  qmlRegisterType<ChannelModel>("harbour.sailpipe.extractor", 1, 0, "ChannelModel");
  qmlRegisterType<CommentModel>("harbour.sailpipe.extractor", 1, 0, "CommentModel");
  qmlRegisterType<PlaylistModel>("harbour.sailpipe.extractor", 1, 0, "PlaylistModel");
  qmlRegisterType<MediaInfo>("harbour.sailpipe.extractor", 1, 0, "MediaInfo");
  qmlRegisterType<PageRef>("harbour.sailpipe.extractor", 1, 0, "PageRef");
  qmlRegisterType<SearchItem>("harbour.sailpipe.extractor", 1, 0, "SearchItem");
  qmlRegisterType<ChannelInfo>("harbour.sailpipe.extractor", 1, 0, "ChannelInfo");
  qmlRegisterType<ChannelTabInfo>("harbour.sailpipe.extractor", 1, 0, "ChannelTabInfo");
  qmlRegisterType<ListLinkHandler>("harbour.sailpipe.extractor", 1, 0, "ListLinkHandler");
  qmlRegisterType<LinkHandlerModel>("harbour.sailpipe.extractor", 1, 0, "LinkHandlerModel");
  qmlRegisterType<ChannelTabListModel>("harbour.sailpipe.extractor", 1, 0, "ChannelTabListModel");
  qmlRegisterType<FilterModel>("harbour.sailpipe.extractor", 1, 0, "FilterModel");
  qmlRegisterSingletonType<Utils>("harbour.sailpipe.extractor", 1, 0, "Utils", Utils::provider);
  qmlRegisterSingletonType<DownloadManager>("harbour.sailpipe.extractor", 1, 0, "DownloadManager", DownloadManager::provider);
  qmlRegisterSingletonType<MediaJunction>("harbour.sailpipe.extractor", 1, 0, "MediaJunction", MediaJunction::provider);
  qmlRegisterSingletonType<YtDlpManager>("harbour.sailpipe.extractor", 1, 0, "YtDlp", YtDlpManager::provider);

  engine->addImageProvider(QLatin1String("sailpipe"), new ImageProvider());

  ctxt->setContextProperty("extractor", extractor.data());
  ctxt->setContextProperty("sailpipeVersion", SAILPIPE_VERSION);
  qDebug() << "harbour-sailreel VERSION string: " << SAILPIPE_VERSION;

  view->setSource(SailfishApp::pathTo("qml/harbour-sailreel.qml"));
  view->show();
  result = app->exec();

  return result;
}
