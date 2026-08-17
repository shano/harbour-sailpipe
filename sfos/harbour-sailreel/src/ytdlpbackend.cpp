#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

#include "ytdlpprocess.h"
#include "ytdlptranslate.h"
#include "ytdlpbackend.h"

#define SEARCH_PAGE_SIZE 20
#define COMMENT_PAGE_SIZE 20
#define CHANNEL_PAGE_SIZE 30
#define PLAYLIST_PAGE_SIZE 30
#define SUBSCRIPTION_FEED_PER_CHANNEL 10

static QJsonDocument errorResult(QString const& message)
{
  QJsonObject obj;
  obj[QStringLiteral("error")] = message;
  return QJsonDocument(obj);
}

int YtDlpBackend::pageOffset(QJsonObject const& in)
{
  QJsonObject page = in[QStringLiteral("page")].toObject();
  bool ok = false;
  int offset = page[QStringLiteral("id")].toString().toInt(&ok);
  return ok ? offset : 0;
}

QJsonDocument YtDlpBackend::invoke(QString const& methodName, QJsonDocument const& in)
{
  QJsonObject inObject = in.object();
  QJsonObject result;

  if (methodName == QStringLiteral("searchFor")) {
    return searchFor(inObject);
  }
  else if (methodName == QStringLiteral("getMoreSearchItems")) {
    return getMoreSearchItems(inObject);
  }
  else if (methodName == QStringLiteral("downloadExtract")) {
    return downloadExtract(inObject);
  }
  else if (methodName == QStringLiteral("getCommentsInfo")) {
    return getCommentsInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMoreCommentItems")) {
    return getMoreCommentItems(inObject);
  }
  else if (methodName == QStringLiteral("getChannelInfo")) {
    return getChannelInfo(inObject);
  }
  else if (methodName == QStringLiteral("getChannelTabInfo")) {
    return getChannelTabInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMoreChannelItems")) {
    return getMoreChannelItems(inObject);
  }
  else if (methodName == QStringLiteral("getPlaylistInfo")) {
    return getPlaylistInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMorePlaylistItems")) {
    return getMorePlaylistItems(inObject);
  }
  else if (methodName == QStringLiteral("getSubscriptionFeed")) {
    return getSubscriptionFeed(inObject);
  }
  else if (methodName == QStringLiteral("tearDown")) {
    return QJsonDocument(QJsonObject());
  }

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::searchFor(QJsonObject const& in)
{
  QString searchTerm = in[QStringLiteral("searchString")].toString();
  int requested = SEARCH_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << QString("ytsearch%1:%2").arg(requested).arg(searchTerm));

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, SEARCH_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreSearchItems(QJsonObject const& in)
{
  QString searchTerm = in[QStringLiteral("searchString")].toString();
  int offset = pageOffset(in);
  int requested = offset + SEARCH_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << QString("ytsearch%1:%2").arg(requested).arg(searchTerm));

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, SEARCH_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::downloadExtract(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("-f")
    << QStringLiteral("best[ext=mp4]/best")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonObject result = YtDlpTranslate::mediaInfo(process.output.object());
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getCommentsInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--write-comments")
    << QStringLiteral("--extractor-args")
    << QStringLiteral("youtube:max_comments=100")
    << QStringLiteral("-J")
    << QStringLiteral("--no-download")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray comments = process.output.object()[QStringLiteral("comments")].toArray();
  QJsonObject result = YtDlpTranslate::commentResults(comments, 0, COMMENT_PAGE_SIZE);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreCommentItems(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();
  int offset = pageOffset(in);

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--write-comments")
    << QStringLiteral("--extractor-args")
    << QStringLiteral("youtube:max_comments=100")
    << QStringLiteral("-J")
    << QStringLiteral("--no-download")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray comments = process.output.object()[QStringLiteral("comments")].toArray();
  QJsonObject result = YtDlpTranslate::commentResults(comments, offset, COMMENT_PAGE_SIZE);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getChannelInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-items")
    << QStringLiteral("0")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonObject info = process.output.object();
  QJsonObject result = YtDlpTranslate::channelInfo(info);

  // yt-dlp's flat-playlist on a bare channel URL returns one entry per
  // channel tab (Videos/Live/Shorts as pseudo-playlists), not the videos
  // within a tab — the "/videos" suffix is required to list actual videos.
  QString channelUrl = result[QStringLiteral("url")].toString();
  QString videosUrl = channelUrl.endsWith(QLatin1Char('/'))
    ? channelUrl + QStringLiteral("videos")
    : channelUrl + QStringLiteral("/videos");

  QJsonObject videosTab;
  videosTab[QStringLiteral("originalUrl")] = videosUrl;
  videosTab[QStringLiteral("url")] = videosUrl;
  videosTab[QStringLiteral("id")] = QStringLiteral("videos");
  videosTab[QStringLiteral("contentFilters")] = QJsonArray{QStringLiteral("videos")};
  videosTab[QStringLiteral("sortFilter")] = QString();

  QJsonArray tabs;
  tabs.append(videosTab);
  result[QStringLiteral("tabs")] = tabs;

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getChannelTabInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-end") << QString::number(CHANNEL_PAGE_SIZE)
    << QStringLiteral("-J")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, CHANNEL_PAGE_SIZE, CHANNEL_PAGE_SIZE);
  result[QStringLiteral("contentFilters")] = QJsonArray{QStringLiteral("videos")};
  result[QStringLiteral("sortFilter")] = QString();

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreChannelItems(QJsonObject const& in)
{
  QJsonObject linkHandler = in;
  QString url = linkHandler[QStringLiteral("url")].toString();
  int offset = pageOffset(in);
  int requested = offset + CHANNEL_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-end") << QString::number(requested)
    << QStringLiteral("-J")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, CHANNEL_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getPlaylistInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-end") << QString::number(PLAYLIST_PAGE_SIZE)
    << QStringLiteral("-J")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonObject info = process.output.object();
  QJsonArray entries = info[QStringLiteral("entries")].toArray();

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, PLAYLIST_PAGE_SIZE, PLAYLIST_PAGE_SIZE);
  QJsonObject extra = YtDlpTranslate::playlistExtra(info);
  for (auto it = extra.constBegin(); it != extra.constEnd(); ++it) {
    result[it.key()] = it.value();
  }

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMorePlaylistItems(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();
  int offset = pageOffset(in);
  int requested = offset + PLAYLIST_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-end") << QString::number(requested)
    << QStringLiteral("-J")
    << url, 60000);

  if (!process.success) {
    return errorResult(process.errorMessage);
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, PLAYLIST_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

namespace {

QJsonArray fetchChannelVideos(QString const& channelUrl)
{
  QJsonArray items;

  QString videosUrl = channelUrl.endsWith(QLatin1Char('/'))
    ? channelUrl + QStringLiteral("videos")
    : channelUrl + QStringLiteral("/videos");

  // Plain --flat-playlist never populates timestamp/upload_date for a
  // channel's video listing (confirmed live) — every entry's uploadDate
  // would be 0, degrading the cross-channel merge sort below into
  // per-channel insertion order (all of one channel, then the next).
  // youtubetab:approximate_date fills in a real (day-granularity)
  // timestamp without the cost of full per-video extraction.
  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--extractor-args") << QStringLiteral("youtubetab:approximate_date")
    << QStringLiteral("--playlist-end") << QString::number(SUBSCRIPTION_FEED_PER_CHANNEL)
    << QStringLiteral("-J")
    << videosUrl, 60000);

  // A single subscribed channel failing to fetch (deleted, private, network
  // blip) shouldn't take down the whole aggregate feed — it just
  // contributes no items.
  if (process.success) {
    QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
    for (QJsonValue const& entry : entries) {
      items.append(YtDlpTranslate::streamItem(entry.toObject()));
    }
  }

  return items;
}

qint64 uploadEpoch(QJsonObject const& item)
{
  return static_cast<qint64>(item[QStringLiteral("uploadDate")].toObject()[QStringLiteral("offsetDateTime")].toDouble());
}

} // namespace

QJsonDocument YtDlpBackend::getSubscriptionFeed(QJsonObject const& in)
{
  QJsonArray channelUrls = in[QStringLiteral("channelUrls")].toArray();

  // Each subscribed channel is a separate yt-dlp subprocess invocation;
  // running them concurrently keeps the wait roughly at the slowest single
  // channel rather than the sum of all of them.
  QVector<QFuture<QJsonArray>> futures;
  for (QJsonValue const& urlValue : channelUrls) {
    QString channelUrl = urlValue.toString();
    futures.append(QtConcurrent::run(fetchChannelVideos, channelUrl));
  }

  QJsonArray allItems;
  for (QFuture<QJsonArray>& future : futures) {
    future.waitForFinished();
    for (QJsonValue const& item : future.result()) {
      allItems.append(item);
    }
  }

  QVector<QJsonObject> sortable;
  sortable.reserve(allItems.size());
  for (QJsonValue const& item : allItems) {
    sortable.append(item.toObject());
  }
  std::stable_sort(sortable.begin(), sortable.end(), [](QJsonObject const& a, QJsonObject const& b) {
    return uploadEpoch(a) > uploadEpoch(b);
  });

  QJsonArray sortedItems;
  for (QJsonObject const& item : sortable) {
    sortedItems.append(item);
  }

  QJsonObject result;
  result[QStringLiteral("relatedItems")] = sortedItems;
  result[QStringLiteral("itemsList")] = sortedItems;
  result[QStringLiteral("nextPage")] = QJsonObject();

  return QJsonDocument(result);
}
