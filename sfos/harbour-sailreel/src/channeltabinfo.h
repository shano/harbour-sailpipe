#ifndef CHANNELTABINFO_H
#define CHANNELTABINFO_H

#include <QObject>

#include "searchitem.h"
#include "pageref.h"

class ChannelTabInfo : public QObject
{
  Q_OBJECT
public:
  explicit ChannelTabInfo(QObject *parent = nullptr);
  explicit ChannelTabInfo(QList<SearchItem const*> relatedItems, PageRef* nextPage, QStringList const& contentFilters, QString const& sortFilter, QObject *parent = nullptr);
  explicit ChannelTabInfo(QJsonObject const& json, QObject *parent = nullptr);

  QList<SearchItem const*> relatedItems() const;
  PageRef* nextPage() const;
  QStringList contentFilters() const;
  QString sortFilter() const;

  void setRelatedItems(QList<SearchItem const*> const& relatedItems);
  void setPage(PageRef* nextPage);
  void setContentFilters(QStringList const& contentFilters);
  void setSortFilter(QString const& sortFilter);

  void parseJson(QJsonObject const& json);

signals:

private:
  QList<SearchItem const*> m_relatedItems;
  PageRef* m_nextPage;
  QStringList m_contentFilters;
  QString m_sortFilter;
};

#endif // CHANNELTABINFO_H




