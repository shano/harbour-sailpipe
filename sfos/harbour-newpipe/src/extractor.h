#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <QObject>
#include <QFuture>
#include <QThreadPool>

#include "appwrapper.h"

class SearchModel;
class CommentModel;
class PlaylistModel;
class MediaInfo;
class PageRef;
class ChannelInfo;
class ChannelTabInfo;
class ListLinkHandler;
class LinkHandlerModel;
class FilterModel;

class Extractor : public QObject
{
  Q_PROPERTY(Service service READ service WRITE setService NOTIFY serviceChanged)
  Q_PROPERTY(QString serviceName READ serviceName NOTIFY serviceChanged)

  Q_OBJECT
public:
  enum Service {
    ServiceInvalid = -1,

    YouTubeService = 0,
    SoundcloudService= 1,
    MediaCCCService = 2,
    PeertubeService = 3,
    BandcampService = 4,

    ServiceNum
  };
  Q_ENUM(Service)

  explicit Extractor(QObject *parent = nullptr);
  ~Extractor();

public slots:
  void search(SearchModel* searchModel, QString const& searchTerm, QStringList const& contentFilter, QString const& sortFilter);
  void searchMore(SearchModel* searchModel, QString const& searchTerm, QStringList const& contentFilter, QString const& sortFilter, PageRef* page);
  void downloadExtract(MediaInfo* mediaInfo, QString const& url);
  void getComments(CommentModel* commentModel, QString const& url);
  void getMoreComments(CommentModel* commentModel, QString const& url, PageRef* page);
  void appendMoreComments(CommentModel* commentModel, QString const& url, PageRef* page);
  void getChannelInfo(ChannelInfo* channelInfo, LinkHandlerModel* linkHandlerModel, QString const& url);
  void getChannelTabInfo(ChannelTabInfo* channelTabInfo, ListLinkHandler* linkHandler, SearchModel* videoModel);
  void getMoreChannelItems(ListLinkHandler* linkHandler, PageRef* page, SearchModel* videoModel);
  void getPlaylistInfo(PlaylistModel* playlistModel, QString const& url);
  void getMorePlaylistItems(PlaylistModel* playlistModel, QString const& url, PageRef* page);
  void getAvailableContentFilter(FilterModel* filterModel);

  static QString serviceToString(Service service);
  Service service() const;
  void setService(Service service);
  QString serviceName() const;

signals:
  void extracted(QString const& url);
  void serviceChanged();

private:
  QJsonDocument invokeSync(QString const methodName, QJsonDocument const* in);

  QFuture<QJsonDocument> invokeAsync(QString const methodName, QJsonDocument const* in);

public:
  graal_isolate_t* m_isolate;
  graal_isolatethread_t* m_thread;
  QThreadPool m_threadPool;
  Service m_service;
};

#endif // EXTRACTOR_H
