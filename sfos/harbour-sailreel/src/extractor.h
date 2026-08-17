#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <QObject>
#include <QFuture>
#include <QThreadPool>

class SearchModel;
class CommentModel;
class PlaylistModel;
class MediaInfo;
class PageRef;
class ChannelInfo;
class ChannelTabInfo;
class ListLinkHandler;
class LinkHandlerModel;

class Extractor : public QObject
{
  Q_OBJECT
public:
  explicit Extractor(QObject *parent = nullptr);

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
  void getSubscriptionFeed(SearchModel* feedModel, QStringList const& channelUrls);

signals:
  void extracted(QString const& url);
  void errorOccurred(QString message);

private:
  QJsonDocument invokeSync(QString const methodName, QJsonDocument const* in);

  QFuture<QJsonDocument> invokeAsync(QString const methodName, QJsonDocument const* in);

  void emitErrorIfPresent(QJsonDocument const& result);

  QThreadPool m_threadPool;
};

#endif // EXTRACTOR_H
