#ifndef YTDLPBACKEND_H
#define YTDLPBACKEND_H

#include <QJsonDocument>
#include <QString>

class YtDlpBackend
{
public:
  static QJsonDocument invoke(QString const& methodName, QJsonDocument const& in);

private:
  static QJsonDocument searchFor(QJsonObject const& in);
  static QJsonDocument getMoreSearchItems(QJsonObject const& in);
  static QJsonDocument downloadExtract(QJsonObject const& in);
  static QJsonDocument getCommentsInfo(QJsonObject const& in);
  static QJsonDocument getMoreCommentItems(QJsonObject const& in);
  static QJsonDocument getChannelInfo(QJsonObject const& in);
  static QJsonDocument getChannelTabInfo(QJsonObject const& in);
  static QJsonDocument getMoreChannelItems(QJsonObject const& in);
  static QJsonDocument getPlaylistInfo(QJsonObject const& in);
  static QJsonDocument getMorePlaylistItems(QJsonObject const& in);
  static int pageOffset(QJsonObject const& in);
};

#endif // YTDLPBACKEND_H
