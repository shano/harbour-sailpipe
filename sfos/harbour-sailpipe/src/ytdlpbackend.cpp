#include <QJsonObject>
#include <QJsonArray>

#include "ytdlpprocess.h"
#include "ytdlptranslate.h"
#include "ytdlpbackend.h"

#define SEARCH_PAGE_SIZE 20

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
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getCommentsInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMoreCommentItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getChannelInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getChannelTabInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMoreChannelItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
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
