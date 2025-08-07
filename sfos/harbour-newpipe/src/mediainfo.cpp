#include <QJsonObject>
#include <QDebug>

#include "utils.h"

#include "mediainfo.h"

MediaInfo::MediaInfo(QObject *parent)
  : QObject(parent)
  , m_name()
  , m_uploaderName()
  , m_category()
  , m_viewCount(0)
  , m_likeCount(0)
  , m_content()
{
}

MediaInfo::MediaInfo(QString const& url, QString const& name, QString const& uploaderName, QString const& category, qint64 viewCount, qint64 likeCount, QString const& content, QDateTime const& uploadDate, QString description, DescType descriptionType, quint64 length, QString licence, QObject *parent)
  : QObject(parent)
  , m_name(name)
  , m_uploaderName(uploaderName)
  , m_category(category)
  , m_viewCount(viewCount)
  , m_likeCount(likeCount)
  , m_content(content)
  , m_uploadDate(uploadDate)
  , m_description(description)
  , m_descriptionType(descriptionType)
  , m_length(length)
  , m_licence(licence)
{

}

MediaInfo::MediaInfo(QJsonObject const& json, QObject *parent)
  : MediaInfo(parent)
{
  parseJsonChanges(json);
}

QString MediaInfo::name() const
{
  return m_name;
}

QString MediaInfo::uploaderName() const
{
  return m_uploaderName;
}

QString MediaInfo::category() const
{
  return m_category;
}

qint64 MediaInfo::viewCount() const
{
  return m_viewCount;
}

qint64 MediaInfo::likeCount() const
{
  return m_likeCount;
}

QString MediaInfo::content() const
{
  return m_content;
}

QDateTime MediaInfo::uploadDate() const
{
  return m_uploadDate;
}

QString MediaInfo::description() const
{
  return m_description;
}

MediaInfo::DescType MediaInfo::descriptionType() const
{
  return m_descriptionType;
}

quint64 MediaInfo::length() const
{
  return m_length;
}

QString MediaInfo::licence() const
{
  return m_licence;
}

void MediaInfo::setName(QString const& name)
{
  if (m_name != name) {
    m_name = name;
    emit nameChanged();
  }
}

void MediaInfo::setUploaderName(QString const& uploaderName)
{
  if (m_uploaderName != uploaderName) {
    m_uploaderName = uploaderName;
    emit uploaderNameChanged();
  }
}

void MediaInfo::setCategory(QString const& category)
{
  if (m_category != category) {
    m_category = category;
    emit categoryChanged();
  }
}

void MediaInfo::setViewCount(qint64 viewCount)
{
  if (m_viewCount != viewCount) {
    m_viewCount = viewCount;
    emit viewCountChanged();
  }
}

void MediaInfo::setLikeCount(qint64 likeCount)
{
  if (m_likeCount != likeCount) {
    m_likeCount = likeCount;
    emit likeCountChanged();
  }
}

void MediaInfo::setContent(QString const& content)
{
  if (m_content != content) {
    m_content = content;
    emit contentChanged();
  }
}

void MediaInfo::setUploadDate(QDateTime const& uploadDate)
{
  if (m_uploadDate != uploadDate) {
    m_uploadDate = uploadDate;
    emit uploadDateChanged();
  }
}

void MediaInfo::setDescription(QString const& description)
{
  if (m_description != description) {
    m_description = description;
    emit descriptionChanged();
  }
}

void MediaInfo::setDescriptionType(DescType descriptionType)
{
  if (m_descriptionType != descriptionType) {
    m_descriptionType = descriptionType;
    emit descriptionTypeChanged();
  }
}

void MediaInfo::setLength(quint64 length)
{
  if (m_length != length) {
    m_length = length;
    emit lengthChanged();
  }
}

void MediaInfo::setLicence(QString const& licence)
{
  if (m_licence != licence) {
    m_licence = licence;
    emit licenceChanged();
  }
}

void MediaInfo::parseJson(QJsonObject const& json)
{
  QList<MediaInfoSignal> emissions;
  emissions = parseJsonChanges(json);

  for (void (MediaInfo::*emission)() : emissions) {
    emit (this->*emission)();
  }
}

QList<MediaInfo::MediaInfoSignal> MediaInfo::parseJsonChanges(QJsonObject const& json)
{
  QList<MediaInfoSignal> emissions;
  QString name;
  QString uploaderName;
  QString category;
  qint64 viewCount;
  qint64 likeCount;
  QString content;
  QDateTime uploadDate;
  QString description;
  DescType descriptionType;
  quint64 length;
  QString licence;

  name = json["name"].toString();
  if (m_name != name) {
    m_name = name;
    emissions << &MediaInfo::nameChanged;
  }
  uploaderName = json["uploaderName"].toString();
  if (m_uploaderName != uploaderName) {
    m_uploaderName = uploaderName;
    emissions << &MediaInfo::uploaderNameChanged;
  }
  category = json["category"].toString();
  if (m_category != category) {
    m_category = category;
    emissions << &MediaInfo::categoryChanged;
  }
  viewCount = json["viewCount"].toInt();
  if (m_viewCount != viewCount) {
    m_viewCount = viewCount;
    emissions << &MediaInfo::viewCountChanged;
  }
  likeCount = json["likeCount"].toInt();
  if (m_likeCount != likeCount) {
    m_likeCount = likeCount;
    emissions << &MediaInfo::likeCountChanged;
  }
  content = json["content"].toString();
  if (m_content != content) {
    m_content = content;
    emissions << &MediaInfo::contentChanged;
  }
  uploadDate = Utils::epochToDateTime(json["uploadDate"].toInt());
  if (m_uploadDate != uploadDate) {
    m_uploadDate = uploadDate;
    emissions << &MediaInfo::uploadDateChanged;
  }
  description = json["description"].toObject()["content"].toString();
  if (m_description != description) {
    m_description = description;
    emissions << &MediaInfo::descriptionChanged;
  }
  descriptionType = static_cast<DescType>(json["description"].toObject()["type"].toInt());
  if (m_descriptionType != descriptionType) {
    m_descriptionType = descriptionType;
    emissions << &MediaInfo::descriptionTypeChanged;
  }
  length = json["length"].toInt();
  if (m_length != length) {
    m_length = length;
    emissions << &MediaInfo::lengthChanged;
  }
  licence = json["licence"].toString();
  if (m_licence != licence) {
    m_licence = licence;
    emissions << &MediaInfo::licenceChanged;
  }

  return emissions;
}



