#ifndef YTDLPTRANSLATE_H
#define YTDLPTRANSLATE_H

#include <QJsonObject>
#include <QJsonArray>

namespace YtDlpTranslate {

QJsonObject streamItem(QJsonObject const& entry);
QJsonObject channelItem(QJsonObject const& entry);
QJsonObject searchResults(QJsonArray const& entries, int offset, int pageSize, int totalRequested);
QJsonObject mediaInfo(QJsonObject const& info);
QJsonObject commentItem(QJsonObject const& comment);
QJsonObject commentResults(QJsonArray const& comments, int offset, int pageSize);
QJsonObject channelInfo(QJsonObject const& info);
QJsonObject playlistExtra(QJsonObject const& info);

}

#endif // YTDLPTRANSLATE_H
