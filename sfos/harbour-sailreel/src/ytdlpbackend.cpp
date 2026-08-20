#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

#include "utils.h"
#include "ytdlpprocess.h"
#include "ytdlptranslate.h"
#include "ytdlpbackend.h"

#define SEARCH_PAGE_SIZE 20
#define COMMENT_PAGE_SIZE 20
#define CHANNEL_PAGE_SIZE 30
#define PLAYLIST_PAGE_SIZE 30
#define SUBSCRIPTION_FEED_PER_CHANNEL 10
#define SUBSCRIPTION_CACHE_TTL_SECS 3600

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

  // android_vr currently requires a GVS PO token for any format other than
  // 18, and even format 18 has been intermittently 403ing lately
  // (confirmed on-device, matches yt-dlp/yt-dlp#17348 and #16150) — exclude
  // it and let yt-dlp's normal client fallback chain pick a working one.
  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("-f")
    << QStringLiteral("best[ext=mp4]/best")
    << QStringLiteral("--extractor-args") << QStringLiteral("youtube:player_client=all,-android_vr")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    Utils::logDebug(QStringLiteral("downloadExtract failed for %1: %2").arg(url, process.errorMessage));
    return errorResult(process.errorMessage);
  }

  QJsonObject rawInfo = process.output.object();
  Utils::logDebug(QStringLiteral("downloadExtract succeeded for %1: format_id=%2 ext=%3 protocol=%4 url=%5")
    .arg(url, rawInfo[QStringLiteral("format_id")].toString(), rawInfo[QStringLiteral("ext")].toString(),
         rawInfo[QStringLiteral("protocol")].toString(), rawInfo[QStringLiteral("url")].toString()));

  QJsonObject result = YtDlpTranslate::mediaInfo(rawInfo);
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

QString subscriptionCachePath()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return dataDir + QStringLiteral("/subscriptions_cache.json");
}

// Returns a null array if there's no cache, or it's older than
// SUBSCRIPTION_CACHE_TTL_SECS.
QJsonArray readSubscriptionCache()
{
  QFile file(subscriptionCachePath());
  if (!file.open(QIODevice::ReadOnly)) {
    return QJsonArray();
  }

  QJsonObject cache = QJsonDocument::fromJson(file.readAll()).object();
  qint64 cachedAt = static_cast<qint64>(cache[QStringLiteral("cachedAt")].toDouble());
  qint64 age = QDateTime::currentSecsSinceEpoch() - cachedAt;
  if (age > SUBSCRIPTION_CACHE_TTL_SECS) {
    return QJsonArray();
  }

  return cache[QStringLiteral("items")].toArray();
}

void writeSubscriptionCache(QJsonArray const& items)
{
  QString path = subscriptionCachePath();
  QDir().mkpath(QFileInfo(path).absolutePath());

  QJsonObject cache;
  cache[QStringLiteral("cachedAt")] = QDateTime::currentSecsSinceEpoch();
  cache[QStringLiteral("items")] = items;

  QFile file(path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write(QJsonDocument(cache).toJson(QJsonDocument::Compact));
  }
}

} // namespace

QJsonDocument YtDlpBackend::getSubscriptionFeed(QJsonObject const& in)
{
  QJsonArray channelUrls = in[QStringLiteral("channelUrls")].toArray();
  bool forceRefresh = in[QStringLiteral("forceRefresh")].toBool(false);

  if (!forceRefresh) {
    QJsonArray cached = readSubscriptionCache();
    if (!cached.isEmpty()) {
      QJsonObject cachedResult;
      cachedResult[QStringLiteral("relatedItems")] = cached;
      cachedResult[QStringLiteral("itemsList")] = cached;
      cachedResult[QStringLiteral("nextPage")] = QJsonObject();
      return QJsonDocument(cachedResult);
    }
  }

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

  writeSubscriptionCache(sortedItems);

  QJsonObject result;
  result[QStringLiteral("relatedItems")] = sortedItems;
  result[QStringLiteral("itemsList")] = sortedItems;
  result[QStringLiteral("nextPage")] = QJsonObject();

  return QJsonDocument(result);
}
