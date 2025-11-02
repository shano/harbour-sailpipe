#include <QJsonObject>
#include <QJsonArray>

#include "listlinkhandler.h"

ListLinkHandler::ListLinkHandler(QObject *parent)
  : QObject(parent)
  , m_originalUrl()
  , m_url()
  , m_id()
  , m_contentFilters()
  , m_sortFilter()
{

}

ListLinkHandler::ListLinkHandler(QString const& originalUrl, QString const& url, QString const& id, QStringList const& contentFilters, QString const& sortFilter, QObject *parent)
  : QObject(parent)
  , m_originalUrl(originalUrl)
  , m_url(url)
  , m_id(id)
  , m_contentFilters(contentFilters)
  , m_sortFilter(sortFilter)
{

}

ListLinkHandler::ListLinkHandler(QJsonObject const& json, QObject *parent)
  : ListLinkHandler(parent)
{
  parseJson(json);
}

QString ListLinkHandler::originalUrl() const
{
  return m_originalUrl;
}

QString ListLinkHandler::url() const
{
  return m_url;
}

QString ListLinkHandler::id() const
{
  return m_id;
}

QStringList ListLinkHandler::contentFilters() const
{
  return m_contentFilters;
}

QString ListLinkHandler::sortFilter() const
{
  return m_sortFilter;
}

void ListLinkHandler::setOriginalUrl(QString const& originalUrl)
{
  m_originalUrl = originalUrl;
}

void ListLinkHandler::setUrl(QString const& url)
{
  m_url = url;
}

void ListLinkHandler::setId(QString const& id)
{
  m_id = id;
}

void ListLinkHandler::setContentFilters(QStringList const& contentFilters)
{
  m_contentFilters = contentFilters;
}

void ListLinkHandler::setSortFilter(QString const& sortFilter)
{
  m_sortFilter = sortFilter;
}

QJsonObject ListLinkHandler::toJson() const
{
  QJsonObject json;
  QJsonArray contentFilters;
  QJsonObject cookies;

  json["originalUrl"] = m_originalUrl;
  json["url"] = m_url;
  json["id"] = m_id;

  for (QString const& contentFilter : m_contentFilters) {
    contentFilters.append(contentFilter);
  }
  json["contentFilters"] = contentFilters;

  json["sortFilter"] = m_sortFilter;

  return json;
}

void ListLinkHandler::parseJson(QJsonObject const& json)
{
  m_originalUrl = json["originalUrl"].toString();
  m_url = json["url"].toString();
  m_id = json["id"].toString();

  QJsonArray contentFilters = json["contentFilters"].toArray();
  m_contentFilters.clear();
  for (QJsonValue const& contentFilter : contentFilters) {
    m_contentFilters.append(contentFilter.toString());
  }

  m_sortFilter = json["sortFilter"].toString();
}
