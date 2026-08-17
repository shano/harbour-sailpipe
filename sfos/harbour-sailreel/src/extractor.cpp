#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>
#include <QQmlEngine>

#include "searchmodel.h"
#include "commentmodel.h"
#include "playlistmodel.h"
#include "mediainfo.h"
#include "pageref.h"
#include "lifetimecheck.h"
#include "channelinfo.h"
#include "channeltabinfo.h"
#include "listlinkhandler.h"
#include "linkhandlermodel.h"
#include "ytdlpbackend.h"

#include "extractor.h"

Extractor::Extractor(QObject *parent)
  : QObject(parent)
{
}

QJsonDocument Extractor::invokeSync(QString const methodName, QJsonDocument const* in)
{
  return YtDlpBackend::invoke(methodName, *in);
}

QFuture<QJsonDocument> Extractor::invokeAsync(QString const methodName, QJsonDocument const* in)
{
  QJsonDocument document = in ? *in : QJsonDocument();
  return QtConcurrent::run(&m_threadPool, [methodName, document]() {
    return YtDlpBackend::invoke(methodName, document);
  });
}

void Extractor::emitErrorIfPresent(QJsonDocument const& result)
{
  if (result.isObject()) {
    QJsonObject obj = result.object();
    if (obj.contains(QStringLiteral("error"))) {
      emit errorOccurred(obj[QStringLiteral("error")].toString());
    }
  }
}

void Extractor::search(SearchModel* searchModel, QString const& searchTerm, QStringList const& contentFilter, QString const& sortFilter)
{
  QJsonObject json;
  QJsonArray filters;
  QJsonDocument document;

  json["searchString"] = searchTerm;

  for (QString const& filter : contentFilter) {
    filters.push_back(filter);
  }
  if (filters.empty()) {
    filters.push_back(QStringLiteral("all"));
  }

  json["contentFilter"] = filters;
  json["sortFilter"] = sortFilter;
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(searchModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, lifetimeCheck, searchModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["relatedItems"].toArray();
      QList<SearchItem const*> searchResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), searchModel);
        searchResults.append(deserialised);
      }
      searchModel->replaceAll(searchResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), searchModel);
      searchModel->setNextPage(page);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("searchFor", &document));
}

void Extractor::searchMore(SearchModel* searchModel, QString const& searchTerm, QStringList const& contentFilter, QString const& sortFilter, PageRef* page)
{
  QJsonObject json;
  QJsonArray filters;
  QJsonDocument document;

  json["searchString"] = searchTerm;

  for (QString const& filter : contentFilter) {
    filters.push_back(filter);
  }
  if (filters.empty()) {
    filters.push_back(QStringLiteral("all"));
  }

  json["contentFilter"] = filters;
  json["sortFilter"] = sortFilter;
  json["page"] = page->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(searchModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, lifetimeCheck, searchModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["itemsList"].toArray();
      QList<SearchItem const*> searchResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), searchModel);
        searchResults.append(deserialised);
      }
      searchModel->append(searchResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), searchModel);
      searchModel->setNextPage(page);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getMoreSearchItems", &document));
}

void Extractor::downloadExtract(MediaInfo* mediaInfo, QString const& url)
{
  QJsonObject json;
  QJsonDocument document;

  json["url"] = url;
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(mediaInfo, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, url, mediaInfo, lifetimeCheck]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      mediaInfo->parseJson(result.object());
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("downloadExtract", &document));
}

void Extractor::getComments(CommentModel* commentModel, QString const& url)
{
  QJsonObject json;
  QJsonDocument document;

  json["url"] = url;
  json["page"] = QJsonValue();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(commentModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, commentModel, lifetimeCheck]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["relatedItems"].toArray();
      QList<CommentItem const*> comments;
      for (QJsonValue const& item : items) {
        CommentItem const* deserialised = new CommentItem(item.toObject(), commentModel);
        comments.append(deserialised);
      }
      commentModel->replaceAll(comments);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), commentModel);
      commentModel->setNextPage(page);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getCommentsInfo", &document));
}

void Extractor::getMoreComments(CommentModel* commentModel, QString const& url, PageRef* page)
{
  QJsonObject json;
  QJsonDocument document;

  json["url"] = url;
  json["page"] = page->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(commentModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, commentModel, lifetimeCheck]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["itemsList"].toArray();
      QList<CommentItem const*> comments;
      for (QJsonValue const& item : items) {
        CommentItem const* deserialised = new CommentItem(item.toObject(), commentModel);
        comments.append(deserialised);
      }
      commentModel->replaceAll(comments);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), commentModel);
      commentModel->setNextPage(page);
    }
    delete watcher;
  });
  watcher->setFuture(invokeAsync("getMoreCommentItems", &document));
}

void Extractor::appendMoreComments(CommentModel* commentModel, QString const& url, PageRef* page)
{
  QJsonObject json;
  QJsonDocument document;

  json["url"] = url;
  json["page"] = page->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(commentModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, commentModel, lifetimeCheck]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["itemsList"].toArray();
      QList<CommentItem const*> comments;
      for (QJsonValue const& item : items) {
        CommentItem const* deserialised = new CommentItem(item.toObject(), commentModel);
        comments.append(deserialised);
      }
      commentModel->append(comments);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), commentModel);
      commentModel->setNextPage(page);
    }
    delete watcher;
  });
  watcher->setFuture(invokeAsync("getMoreCommentItems", &document));
}

void Extractor::getChannelInfo(ChannelInfo* channelInfo, LinkHandlerModel* linkHandlerModel, QString const& url)
{
  QJsonObject json;
  QJsonDocument document;

  json["url"] = url;
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(channelInfo, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, channelInfo, lifetimeCheck, linkHandlerModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      channelInfo->parseJson(result.object());
      linkHandlerModel->setModel(channelInfo);
      emit extracted(channelInfo->url());
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getChannelInfo", &document));
}

void Extractor::getChannelTabInfo(ChannelTabInfo* channelTabInfo, ListLinkHandler* linkHandler, SearchModel* videoModel)
{
  QJsonObject json;
  QJsonDocument document;

  json = linkHandler->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeChannelTabInfo = new LifetimeCheck(channelTabInfo, watcher);
  LifetimeCheck* lifetimeCVideoModel = new LifetimeCheck(videoModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, channelTabInfo, lifetimeChannelTabInfo, videoModel, lifetimeCVideoModel]() {
    if (!lifetimeChannelTabInfo->destroyed() && !lifetimeCVideoModel->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      channelTabInfo->parseJson(result.object());

      QStringList const& contentFilters = channelTabInfo->contentFilters();
      if (contentFilters.isEmpty()) {
        videoModel->setContentFilter(QString());
      }
      else {
        videoModel->setContentFilter(contentFilters.first());
      }
      QString const& sortFilter = channelTabInfo->sortFilter();
      videoModel->setSortFilter(sortFilter);

      QJsonArray items = result.object()["relatedItems"].toArray();
      QList<SearchItem const*> searchResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), videoModel);
        searchResults.append(deserialised);
      }
      videoModel->replaceAll(searchResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), videoModel);
      videoModel->setNextPage(page);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getChannelTabInfo", &document));
}

void Extractor::getMoreChannelItems(ListLinkHandler* linkHandler, PageRef* page, SearchModel* videoModel)
{
  QJsonObject json;
  QJsonArray filters;
  QJsonDocument document;

  json = linkHandler->toJson();
  json["page"] = page->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(videoModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, videoModel, lifetimeCheck]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["itemsList"].toArray();
      QList<SearchItem const*> searchResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), videoModel);
        searchResults.append(deserialised);
      }
      videoModel->append(searchResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), videoModel);
      videoModel->setNextPage(page);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getMoreChannelItems", &document));
}

void Extractor::getPlaylistInfo(PlaylistModel* playlistModel, QString const& url)
{
  QJsonObject json;
  QJsonArray filters;
  QJsonDocument document;

  json["url"] = url;
  json["page"] = QJsonValue();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(playlistModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, lifetimeCheck, playlistModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["relatedItems"].toArray();
      QList<SearchItem const*> playlistResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), playlistModel);
        playlistResults.append(deserialised);
      }
      playlistModel->replaceAll(playlistResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), playlistModel);
      playlistModel->setNextPage(page);

      playlistModel->parseJson(result.object());

      playlistModel->calculateDuration();
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getPlaylistInfo", &document));
}

void Extractor::getMorePlaylistItems(PlaylistModel* playlistModel, QString const& url, PageRef* page)
{
  QJsonObject json;
  QJsonArray filters;
  QJsonDocument document;

  json["url"] = url;
  json["page"] = page->toJson();
  document = QJsonDocument(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(playlistModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, lifetimeCheck, playlistModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);
      //qDebug() << "Result: " << result.toJson(QJsonDocument::Indented);

      QJsonArray items = result.object()["itemsList"].toArray();
      QList<SearchItem const*> playlistResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), playlistModel);
        playlistResults.append(deserialised);
      }
      playlistModel->append(playlistResults);
      PageRef* page = new PageRef(result.object()["nextPage"].toObject(), playlistModel);
      playlistModel->setNextPage(page);

      playlistModel->calculateDuration();
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getMorePlaylistItems", &document));
}

void Extractor::getSubscriptionFeed(SearchModel* feedModel, QStringList const& channelUrls)
{
  QJsonObject json;
  QJsonArray urls;
  for (QString const& url : channelUrls) {
    urls.append(url);
  }
  json["channelUrls"] = urls;
  QJsonDocument document(json);

  QFutureWatcher<QJsonDocument>* watcher = new QFutureWatcher<QJsonDocument>();
  LifetimeCheck* lifetimeCheck = new LifetimeCheck(feedModel, watcher);
  QObject::connect(watcher, &QFutureWatcher<QJsonDocument>::finished, [this, watcher, lifetimeCheck, feedModel]() {
    if (!lifetimeCheck->destroyed()) {
      QJsonDocument result = watcher->result();
      emitErrorIfPresent(result);

      QJsonArray items = result.object()["relatedItems"].toArray();
      QList<SearchItem const*> feedResults;
      for (QJsonValue const& item : items) {
        SearchItem const* deserialised = SearchItem::createSearchItem(item.toObject(), feedModel);
        feedResults.append(deserialised);
      }
      feedModel->replaceAll(feedResults);
    }

    delete watcher;
  });
  watcher->setFuture(invokeAsync("getSubscriptionFeed", &document));
}

