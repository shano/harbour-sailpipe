#include <QJsonObject>
#include <QJsonArray>

#include "searchitem.h"

#include "channeltabinfo.h"

ChannelTabInfo::ChannelTabInfo(QObject *parent)
  : QObject(parent)
  , m_relatedItems()
  , m_nextPage()
  , m_contentFilters()
  , m_sortFilter()
{

}

ChannelTabInfo::ChannelTabInfo(QList<SearchItem const*> relatedItems, PageRef* nextPage, QStringList const& contentFilters, QString const& sortFilter, QObject *parent)
  : QObject(parent)
  , m_relatedItems(relatedItems)
  , m_nextPage(nextPage)
  , m_contentFilters(contentFilters)
  , m_sortFilter(sortFilter)
{

}

ChannelTabInfo::ChannelTabInfo(QJsonObject const& json, QObject *parent)
  : ChannelTabInfo(parent)
{
  parseJson(json);
}

QList<SearchItem const*> ChannelTabInfo::relatedItems() const
{
  return m_relatedItems;
}

PageRef* ChannelTabInfo::nextPage() const
{
  return m_nextPage;
}

QStringList ChannelTabInfo::contentFilters() const
{
  return m_contentFilters;
}

QString ChannelTabInfo::sortFilter() const
{
  return m_sortFilter;
}

void ChannelTabInfo::setRelatedItems(QList<SearchItem const*> const& relatedItems)
{
  m_relatedItems = relatedItems;
}

void ChannelTabInfo::setPage(PageRef* nextPage)
{
  m_nextPage = nextPage;
}

void ChannelTabInfo::setContentFilters(QStringList const& contentFilters)
{
  m_contentFilters = contentFilters;
}

void ChannelTabInfo::setSortFilter(QString const& sortFilter)
{
  m_sortFilter = sortFilter;
}

void ChannelTabInfo::parseJson(QJsonObject const& json)
{
  m_relatedItems.clear();
  QJsonArray relatedItems = json["relatedItems"].toArray();
  for (QJsonValue const& relatedItem : relatedItems) {
    SearchItem const* deserialised = SearchItem::createSearchItem(relatedItem.toObject(), this);
    m_relatedItems.append(deserialised);
  }

  if (m_nextPage) {
    free(m_nextPage);
  }
  m_nextPage = new PageRef(json["nextPage"].toObject(), this);

  m_contentFilters.clear();
  QJsonArray contentFilters = json["contentFilters"].toArray();
  for (QJsonValue const& contentFilter : contentFilters) {
    QString deserialised = contentFilter.toString();
    m_contentFilters.append(deserialised);
  }

  m_sortFilter = json["sortFilter"].toString();
}
