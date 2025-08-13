#include <QJsonObject>
#include <QJsonArray>
#include <cmath>

#include "playlistmodel.h"
#include "pageref.h"
#include "extractor.h"

PlaylistModel::PlaylistModel(QObject *parent)
  : SearchModel(parent)
  , m_url()
  , m_description()
  , m_descriptionType(PlainText)
  , m_streamCount(0)
  , m_uploaderName()
  , m_uploaderAvatar()
  , m_duration(0)
{
}

void PlaylistModel::search(Extractor* extractor)
{
  extractor->getPlaylistInfo(this, m_url);
}

void PlaylistModel::searchMore(Extractor* extractor)
{
  if (m_more) {
    if (m_nextPage && !(m_nextPage->id().isEmpty() && m_nextPage->ids().empty())) {
      setLoading(true);
      extractor->getMorePlaylistItems(this, m_url, m_nextPage);
    }
  }
}

QString PlaylistModel::url() const
{
  return m_url;
}

QString PlaylistModel::description() const
{
  return m_description;
}

PlaylistModel::DescType PlaylistModel::descriptionType() const
{
  return m_descriptionType;
}

qint64 PlaylistModel::streamCount() const
{
  return m_streamCount;
}

QString PlaylistModel::uploaderName() const
{
  return m_uploaderName;
}

QString PlaylistModel::uploaderAvatar() const
{
  return m_uploaderAvatar;
}

quint64 PlaylistModel::duration() const
{
  return m_duration;
}

void PlaylistModel::setUrl(QString const& url)
{
  if (m_url != url) {
    m_url = url;

    emit urlChanged();
  }
}

void PlaylistModel::setDescription(QString const& description)
{
  if (m_description != description) {
    m_description = description;

    emit descriptionChanged();
  }
}

void PlaylistModel::setDescriptionType(DescType descriptionType)
{
  if (m_descriptionType != descriptionType) {
    m_descriptionType = descriptionType;

    emit descriptionTypeChanged();
  }
}

void PlaylistModel::setStreamCount(qint64 streamCount)
{
  if (m_streamCount != streamCount) {
    m_streamCount = streamCount;

    emit streamCountChanged();
  }
}

void PlaylistModel::setUploaderName(QString const& uploaderName)
{
  if (m_uploaderName != uploaderName) {
    m_uploaderName = uploaderName;

    emit uploaderNameChanged();
  }
}

void PlaylistModel::setUploaderAvatar(QString const& uploaderAvatar)
{
  if (m_uploaderAvatar != uploaderAvatar) {
    m_uploaderAvatar = uploaderAvatar;

    emit uploaderAvatarChanged();
  }
}

void PlaylistModel::calculateDuration()
{
  quint64 duration = 0;

  for (SearchItem const* searchItem : m_searchResults) {
    duration += searchItem->duration();
  }

  if (m_duration != duration) {
    m_duration = duration;

    emit durationChanged();
  }
}

void PlaylistModel::parseJson(QJsonObject const& json)
{
  QList<PlaylistModelSignal> emissions;
  emissions = parseJsonChanges(json);

  for (void (PlaylistModel::*emission)() : emissions) {
    emit (this->*emission)();
  }
}

QList<PlaylistModel::PlaylistModelSignal> PlaylistModel::parseJsonChanges(QJsonObject const& json)
{
  QList<PlaylistModelSignal> emissions;
  QString description;
  DescType descriptionType;
  qint64 streamCount;
  QString uploaderName;
  QString uploaderAvatar;

  description = json["description"].toObject()["content"].toString();
  if (m_description != description) {
    m_description = description;
    emissions << &PlaylistModel::descriptionChanged;
  }
  descriptionType = static_cast<DescType>(json["description"].toObject()["type"].toInt());
  if (m_descriptionType != descriptionType) {
    m_descriptionType = descriptionType;
    emissions << &PlaylistModel::descriptionTypeChanged;
  }

  streamCount = json["streamCount"].toInt();
  if (m_streamCount != streamCount) {
    m_streamCount = streamCount;
    emissions << &PlaylistModel::streamCountChanged;
  }

  uploaderName = json["uploaderName"].toString();
  if (m_uploaderName != uploaderName) {
    m_uploaderName = uploaderName;
    emissions << &PlaylistModel::uploaderNameChanged;
  }

  const int idealWidth = 64;
  const int idealHeight = 64;
  QString uploaderAvatarUrl;
  QJsonArray uploaderAvatars = json["uploaderAvatars"].toArray();
  float minRmse = -1.0;
  for (QJsonValue const& uploaderAvatar : uploaderAvatars) {
    QJsonObject details = uploaderAvatar.toObject();
    int width = details["width"].toInt();
    int height = details["height"].toInt();
    float rmse = std::pow(std::pow(idealWidth - width, 2) + std::pow(idealHeight - height, 2), 0.5f);
    if ((minRmse < 0) || (rmse < minRmse)) {
      uploaderAvatarUrl = details["url"].toString();
      minRmse = rmse;
    }
  }
  m_uploaderAvatar = uploaderAvatarUrl;

  return emissions;
}
