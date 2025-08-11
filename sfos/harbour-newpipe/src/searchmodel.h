#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractListModel>

#include "searchitem.h"

class PageRef;
class Extractor;
class ListLinkHandler;

class SearchModel : public QAbstractListModel
{
  Q_PROPERTY(PageRef* nextPage READ nextPage WRITE setNextPage NOTIFY nextPageChanged)
  Q_PROPERTY(bool loading READ loading WRITE setLoading NOTIFY loadingChanged)
  Q_PROPERTY(bool more READ more WRITE setMore NOTIFY moreChanged)
  Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
  Q_PROPERTY(QString contentFilter READ contentFilter WRITE setContentFilter NOTIFY contentFilterChanged)

  Q_OBJECT
public:
  enum SearchRoles {
    NameRole = Qt::UserRole + 1,
    ThumbnailRole,
    UrlRole,
    InfoTypeRole,
    InfoRowRole,
  };

  explicit SearchModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const;

  int rowCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

  void replaceAll(QList<SearchItem const*> const& searchResults);

  void append(QList<SearchItem const*> const& searchResults);

public slots:
  virtual void search(Extractor* extractor);
  virtual void searchMore(Extractor* extractor);
  void clear();

  PageRef* nextPage() const;
  void setNextPage(PageRef* nextPage);

  bool loading() const;
  void setLoading(bool loading);

  bool more() const;
  void setMore(bool more);

public:
  QString searchTerm() const;
  void setSearchTerm(QString const& searchTerm);

  QString contentFilter() const;
  void setContentFilter(QString const& contentFilter);

  QString sortFilter() const;
  void setSortFilter(QString const& sortFilter);

signals:
  void nextPageChanged();
  void loadingChanged();
  void moreChanged();
  void searchTermChanged();
  void contentFilterChanged();

protected:
  QHash<int, QByteArray> m_roles;
  QList<SearchItem const*> m_searchResults;
  PageRef* m_nextPage;
  bool m_loading;
  bool m_more;
  QString m_searchTerm;
  QStringList m_contentFilters;
  QString m_sortFilter;
};

#endif // SEARCHMODEL_H
