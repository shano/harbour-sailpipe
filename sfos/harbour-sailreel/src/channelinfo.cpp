#include <QJsonObject>
#include <QJsonArray>

#include "listlinkhandler.h"

#include "channelinfo.h"

ChannelInfo::ChannelInfo(QObject *parent)
  : QObject(parent)
  , m_id()
  , m_name()
  , m_url()
  , m_description()
  , m_subscriberCount(0)
  , m_verified(false)
  , m_tags()
  , m_tabs()
{

}

ChannelInfo::ChannelInfo(QJsonObject const& json, QObject *parent)
  : ChannelInfo(parent)
{
  parseJson(json);
}

QString ChannelInfo::id() const
{
  return m_id;
}

QString ChannelInfo::name() const
{
  return m_name;
}

QString ChannelInfo::url() const
{
  return m_url;
}

QString ChannelInfo::description() const
{
  return m_description;
}

QList<ListLinkHandler*> ChannelInfo::tabs()
{
  return m_tabs;
}

void ChannelInfo::setId(QString const& id)
{
  m_id = id;
}

void ChannelInfo::setName(QString const& name)
{
  m_name = name;
}

void ChannelInfo::setUrl(QString const& url)
{
  m_url = url;
}

void ChannelInfo::setDescription(QString const& description)
{
  if (m_description != description) {
    m_description = description;

    emit descriptionChanged();
  }
}

void ChannelInfo::setTabs(QList<ListLinkHandler*> const& tabs)
{
  m_tabs = tabs;
}

qint64 ChannelInfo::subscriberCount() const
{
  return m_subscriberCount;
}

void ChannelInfo::setSubscriberCount(qint64 subscriberCount)
{
  if (m_subscriberCount != subscriberCount) {
    m_subscriberCount = subscriberCount;

    emit subscriberCountChanged();
  }
}

bool ChannelInfo::verified() const
{
  return m_verified;
}

void ChannelInfo::setVerified(bool verified)
{
  if (m_verified != verified) {
    m_verified = verified;

    emit verifiedChanged();
  }
}

QString ChannelInfo::tags() const
{
  return m_tags;
}

void ChannelInfo::setTags(QString& tags)
{
  if (m_tags != tags) {
    m_tags = tags;

    emit tagsChanged();
  }
}

void ChannelInfo::parseJson(QJsonObject const& json)
{
  m_id = json["id"].toString();
  m_name = json["name"].toString();
  m_url = json["url"].toString();
  m_description = json["description"].toString();
  m_subscriberCount = json["subscriberCount"].toInt();
  m_verified = json["verified"].toBool();

  QJsonArray tags = json["tags"].toArray();
  QStringList tagList;
  for (QJsonValue const& tag : tags) {
    tagList.append(tag.toString());
  }
  m_tags = tagList.join(", ");

  QJsonArray tabs = json["tabs"].toArray();
  m_tabs.clear();
  for (QJsonValue const& tabItem : tabs) {
    ListLinkHandler* tab = new ListLinkHandler(tabItem.toObject(), this);
    m_tabs.append(tab);
  }
}
