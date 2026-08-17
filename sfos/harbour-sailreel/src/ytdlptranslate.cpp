#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDate>

#include "ytdlptranslate.h"

namespace {

qint64 epochFromEntry(QJsonObject const& entry)
{
  if (entry.contains(QStringLiteral("timestamp")) && entry[QStringLiteral("timestamp")].isDouble()) {
    return static_cast<qint64>(entry[QStringLiteral("timestamp")].toDouble());
  }
  QString uploadDate = entry[QStringLiteral("upload_date")].toString();
  if (uploadDate.size() == 8) {
    QDate date = QDate::fromString(uploadDate, QStringLiteral("yyyyMMdd"));
    if (date.isValid()) {
      return QDateTime(date, QTime(0, 0), Qt::UTC).toMSecsSinceEpoch() / 1000;
    }
  }
  return 0;
}

QString urlFromEntry(QJsonObject const& entry)
{
  QString url = entry[QStringLiteral("webpage_url")].toString();
  if (url.isEmpty()) {
    QString id = entry[QStringLiteral("id")].toString();
    if (!id.isEmpty()) {
      url = QString("https://www.youtube.com/watch?v=%1").arg(id);
    }
  }
  return url;
}

QString firstNonEmpty(QString const& first, QString const& second)
{
  return first.isEmpty() ? second : first;
}

} // namespace

namespace YtDlpTranslate {

QJsonObject streamItem(QJsonObject const& entry)
{
  QJsonObject item;
  qint64 epoch = epochFromEntry(entry);

  item[QStringLiteral("infoType")] = QStringLiteral("STREAM");
  item[QStringLiteral("name")] = entry[QStringLiteral("title")].toString();
  item[QStringLiteral("thumbnails")] = entry[QStringLiteral("thumbnails")].toArray();
  item[QStringLiteral("url")] = urlFromEntry(entry);
  item[QStringLiteral("uploaderName")] = firstNonEmpty(
    entry[QStringLiteral("uploader")].toString(),
    entry[QStringLiteral("channel")].toString());

  QJsonObject uploadDate;
  uploadDate[QStringLiteral("offsetDateTime")] = epoch;
  item[QStringLiteral("uploadDate")] = uploadDate;

  item[QStringLiteral("textualUploadDate")] = epoch > 0
    ? QDateTime::fromMSecsSinceEpoch(epoch * 1000, Qt::UTC).date().toString(Qt::ISODate)
    : QString();
  item[QStringLiteral("duration")] = static_cast<qint64>(entry[QStringLiteral("duration")].toDouble(0));

  return item;
}

QJsonObject channelItem(QJsonObject const& entry)
{
  QJsonObject item;

  item[QStringLiteral("infoType")] = QStringLiteral("CHANNEL");
  item[QStringLiteral("name")] = firstNonEmpty(
    entry[QStringLiteral("channel")].toString(),
    entry[QStringLiteral("title")].toString());
  item[QStringLiteral("thumbnails")] = entry[QStringLiteral("thumbnails")].toArray();
  // Unlike a video entry, the channel URL is already correct as-is —
  // urlFromEntry()'s watch?v=<id> fallback would be wrong here, since
  // a channel entry's "id" is a channel id, not a video id.
  item[QStringLiteral("url")] = entry[QStringLiteral("url")].toString();
  item[QStringLiteral("description")] = entry[QStringLiteral("description")].toString();
  item[QStringLiteral("subscriberCount")] = entry[QStringLiteral("channel_follower_count")].toInt(0);
  item[QStringLiteral("streamCount")] = entry[QStringLiteral("playlist_count")].toInt(0);
  item[QStringLiteral("verified")] = entry[QStringLiteral("channel_is_verified")].toBool(false);

  return item;
}

namespace {

QJsonObject searchResultItem(QJsonObject const& entry)
{
  // yt-dlp's flat-playlist search results mix video and channel cards in
  // the same entry list, distinguishable only by ie_key — everything
  // else (channel entries have no "duration", playlist ones no "id"
  // that resolves to a video) is too unreliable to key off directly.
  if (entry[QStringLiteral("ie_key")].toString() == QStringLiteral("YoutubeTab")) {
    return channelItem(entry);
  }
  return streamItem(entry);
}

} // namespace

QJsonObject searchResults(QJsonArray const& entries, int offset, int pageSize, int totalRequested)
{
  QJsonObject result;
  QJsonArray items;

  int end = qMin(entries.size(), offset + pageSize);
  for (int i = offset; i < end; ++i) {
    items.append(searchResultItem(entries[i].toObject()));
  }

  result[QStringLiteral("relatedItems")] = items;
  result[QStringLiteral("itemsList")] = items;

  QJsonObject nextPage;
  bool more = (end < entries.size()) || (entries.size() >= totalRequested);
  if (more) {
    nextPage[QStringLiteral("id")] = QString::number(end);
  }
  result[QStringLiteral("nextPage")] = nextPage;

  return result;
}

QJsonObject mediaInfo(QJsonObject const& info)
{
  QJsonObject result;
  qint64 epoch = epochFromEntry(info);
  QJsonArray categories = info[QStringLiteral("categories")].toArray();

  result[QStringLiteral("name")] = info[QStringLiteral("title")].toString();
  result[QStringLiteral("uploaderName")] = firstNonEmpty(
    info[QStringLiteral("uploader")].toString(),
    info[QStringLiteral("channel")].toString());
  result[QStringLiteral("category")] = categories.isEmpty() ? QString() : categories[0].toString();
  result[QStringLiteral("viewCount")] = info[QStringLiteral("view_count")].toInt(0);
  result[QStringLiteral("likeCount")] = info[QStringLiteral("like_count")].toInt(0);
  result[QStringLiteral("content")] = info[QStringLiteral("url")].toString();
  result[QStringLiteral("uploadDate")] = epoch;

  QJsonObject description;
  description[QStringLiteral("content")] = info[QStringLiteral("description")].toString();
  description[QStringLiteral("type")] = 3; // MediaInfo::PlainText
  result[QStringLiteral("description")] = description;

  result[QStringLiteral("length")] = static_cast<qint64>(info[QStringLiteral("duration")].toDouble(0));
  result[QStringLiteral("licence")] = info[QStringLiteral("license")].toString();

  return result;
}

QJsonObject commentItem(QJsonObject const& comment)
{
  QJsonObject item;

  QJsonObject commentText;
  commentText[QStringLiteral("content")] = comment[QStringLiteral("text")].toString();
  item[QStringLiteral("commentText")] = commentText;

  item[QStringLiteral("uploaderName")] = comment[QStringLiteral("author")].toString();

  QJsonArray avatars;
  QString avatarUrl = comment[QStringLiteral("author_thumbnail")].toString();
  if (!avatarUrl.isEmpty()) {
    QJsonObject avatar;
    avatar[QStringLiteral("url")] = avatarUrl;
    avatars.append(avatar);
  }
  item[QStringLiteral("uploaderAvatars")] = avatars;

  item[QStringLiteral("replyCount")] = comment[QStringLiteral("reply_count")].toInt(0);
  item[QStringLiteral("replies")] = QJsonObject();

  return item;
}

QJsonObject commentResults(QJsonArray const& comments, int offset, int pageSize)
{
  QJsonObject result;
  QJsonArray items;

  int end = qMin(comments.size(), offset + pageSize);
  for (int i = offset; i < end; ++i) {
    items.append(commentItem(comments[i].toObject()));
  }

  result[QStringLiteral("relatedItems")] = items;
  result[QStringLiteral("itemsList")] = items;

  QJsonObject nextPage;
  if (end < comments.size()) {
    nextPage[QStringLiteral("id")] = QString::number(end);
  }
  result[QStringLiteral("nextPage")] = nextPage;

  return result;
}

QJsonObject channelInfo(QJsonObject const& info)
{
  QJsonObject result;

  result[QStringLiteral("id")] = info[QStringLiteral("channel_id")].toString();
  result[QStringLiteral("name")] = firstNonEmpty(
    info[QStringLiteral("channel")].toString(),
    info[QStringLiteral("uploader")].toString());
  result[QStringLiteral("url")] = info[QStringLiteral("channel_url")].toString();
  result[QStringLiteral("description")] = info[QStringLiteral("description")].toString();
  result[QStringLiteral("subscriberCount")] = info[QStringLiteral("channel_follower_count")].toInt(0);
  result[QStringLiteral("verified")] = false;
  result[QStringLiteral("tags")] = QJsonArray();
  result[QStringLiteral("tabs")] = QJsonArray();

  return result;
}

QJsonObject playlistExtra(QJsonObject const& info)
{
  QJsonObject result;

  QJsonObject description;
  description[QStringLiteral("content")] = info[QStringLiteral("description")].toString();
  description[QStringLiteral("type")] = 3; // PlaylistModel::PlainText
  result[QStringLiteral("description")] = description;

  int streamCount = info[QStringLiteral("playlist_count")].toInt(-1);
  if (streamCount < 0) {
    streamCount = info[QStringLiteral("entries")].toArray().size();
  }
  result[QStringLiteral("streamCount")] = streamCount;

  result[QStringLiteral("uploaderName")] = firstNonEmpty(
    info[QStringLiteral("uploader")].toString(),
    info[QStringLiteral("channel")].toString());
  result[QStringLiteral("uploaderAvatars")] = QJsonArray();

  return result;
}

} // namespace YtDlpTranslate
