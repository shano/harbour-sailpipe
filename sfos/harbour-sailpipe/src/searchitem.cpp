#include <QJsonObject>
#include <QJsonArray>
#include <cmath>

#include "extractor.h"
#include "searchitemstream.h"
#include "searchitemplaylist.h"
#include "searchitemchannel.h"
#include "searchitemcomment.h"

#include "searchitem.h"

SearchItem::SearchItem(QObject *parent)
  : QObject(parent)
  , m_infoType(None)
  , m_name()
  , m_thumbnail()
{
}

SearchItem::SearchItem(QString const& name, QString const& thumbnail, QString const& url, QObject *parent)
  : QObject(parent)
  , m_infoType(None)
  , m_name(name)
  , m_thumbnail(thumbnail)
  , m_url(url)
{
}

SearchItem::SearchItem(QJsonObject const& json, QObject *parent)
  : QObject(parent)
  , m_infoType(None)
{
  parseJson(json);
}

SearchItem::InfoType SearchItem::getInfoType() const
{
  return m_infoType;
}

QString SearchItem::getName() const
{
  return m_name;
}

QString SearchItem::getThumbnail() const
{
  return m_thumbnail;
}

QString SearchItem::getUrl() const
{
  return m_url;
}

void SearchItem::setInfoType(SearchItem::InfoType infoType)
{
  m_infoType = infoType;
}

void SearchItem::setName(QString const& name)
{
  m_name = name;
}

void SearchItem::setThumbnail(QString const& thumbnail)
{
  m_thumbnail = thumbnail;
}

void SearchItem::setUrl(QString const& url)
{
  m_url = url;
}

void SearchItem::parseJson(QJsonObject const& json)
{
  const int idealWidth = 96 * 1.5;
  const int idealHeight = 64 * 1.5;
  QString thumbnailUrl;
  QJsonArray thumbnails = json["thumbnails"].toArray();
  float minRmse = -1.0;
  for (QJsonValue const& thumbnail : thumbnails) {
    QJsonObject details = thumbnail.toObject();
    int width = details["width"].toInt();
    int height = details["height"].toInt();
    float rmse = std::pow(std::pow(idealWidth - width, 2) + std::pow(idealHeight - height, 2), 0.5f);
    if ((minRmse < 0) || (rmse < minRmse)) {
      thumbnailUrl = details["url"].toString();
      minRmse = rmse;
    }
  }
  m_name = json["name"].toString();
  m_thumbnail = thumbnailUrl;
  m_url = json["url"].toString();
}

SearchItem* SearchItem::createSearchItem(QJsonObject const& json, QObject *parent)
{
  static const QMap<QString, SearchItem::InfoType> infoTypeConvert = {
    {"STREAM", SearchItem::Stream},
    {"PLAYLIST", SearchItem::Playlist},
    {"CHANNEL", SearchItem::Channel},
  };
  InfoType infoType = infoTypeConvert.value(json["infoType"].toString(""), SearchItem::None);
  SearchItem* result;

  switch (infoType) {
    case Stream:
      result = new SearchItemStream(json, parent);
      break;
    case Playlist:
      result = new SearchItemPlaylist(json, parent);
      break;
    case Channel:
      result = new SearchItemChannel(json, parent);
      break;
    case Comment:
      result = new SearchItemComment(json, parent);
      break;
    default:
      result = new SearchItem(json, parent);
      break;
  }

  return result;
}

QString SearchItem::getInfoRow() const
{
  return QString();
}

quint64 SearchItem::duration() const
{
  return 0;
}
