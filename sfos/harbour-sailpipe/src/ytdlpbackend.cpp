#include <QJsonObject>
#include <QJsonArray>

#include "ytdlpprocess.h"
#include "ytdlptranslate.h"
#include "ytdlpbackend.h"

#define SEARCH_PAGE_SIZE 20
#define COMMENT_PAGE_SIZE 20
#define CHANNEL_PAGE_SIZE 30

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
  else if (methodName == QStringLiteral("getAvailableContentFilter")) {
    return getAvailableContentFilter();
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
    return QJsonDocument(QJsonObject());
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
    return QJsonDocument(QJsonObject());
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
    return QJsonDocument(QJsonObject());
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
    return QJsonDocument(QJsonObject());
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
    return QJsonDocument(QJsonObject());
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
    return QJsonDocument(QJsonObject());
  }

  QJsonObject info = process.output.object();
  QJsonObject result = YtDlpTranslate::channelInfo(info);

  QJsonObject videosTab;
  videosTab[QStringLiteral("originalUrl")] = url;
  videosTab[QStringLiteral("url")] = url;
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
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, CHANNEL_PAGE_SIZE, entries.size());
  result[QStringLiteral("contentFilters")] = QJsonArray{QStringLiteral("videos")};
  result[QStringLiteral("sortFilter")] = QString();

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreChannelItems(QJsonObject const& in)
{
  QJsonObject linkHandler = in;
  QString url = linkHandler[QStringLiteral("url")].toString();
  int offset = pageOffset(in);

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, CHANNEL_PAGE_SIZE, entries.size());
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getPlaylistInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMorePlaylistItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getAvailableContentFilter()
{
  return QJsonDocument(QJsonObject());
}
