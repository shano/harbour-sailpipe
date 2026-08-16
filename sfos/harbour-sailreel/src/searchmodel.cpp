#include "extractor.h"
#include "pageref.h"

#include "searchitemstream.h"
#include "searchitemchannel.h"
#include "searchitemplaylist.h"
#include "searchitemcomment.h"

#include "searchmodel.h"

SearchModel::SearchModel(QObject *parent)
  : QAbstractListModel(parent)
  , m_nextPage()
  , m_loading(false)
  , m_more(true)
  , m_searchTerm()
  , m_contentFilters()
  , m_sortFilter()
{
  m_roles[NameRole] = "name";
  m_roles[ThumbnailRole] = "thumbnail";
  m_roles[UrlRole] = "url";
  m_roles[InfoTypeRole] = "infoType";
  m_roles[InfoRowRole] = "infoRow";

  m_contentFilters.append("all");
}

QHash<int, QByteArray> SearchModel::roleNames() const
{
  return m_roles;
}

int SearchModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent)
  return m_searchResults.count();
}

QVariant SearchModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_searchResults.count())) {
    SearchItem const* searchResult = m_searchResults[index.row()];
    switch (role) {
      case NameRole:
        result = searchResult->getName();
        break;
      case ThumbnailRole:
        result = searchResult->getThumbnail();
        break;
      case UrlRole:
        result = searchResult->getUrl();
        break;
      case InfoTypeRole:
        result = searchResult->getInfoType();
        break;
      case InfoRowRole:
        result = searchResult->getInfoRow();
        break;
    }
  }

  return result;
}

void SearchModel::replaceAll(QList<SearchItem const*> const& searchResults)
{
  beginResetModel();
  for (SearchItem const* searchItem : m_searchResults) {
    delete searchItem;
  }
  m_searchResults.clear();
  m_searchResults.append(searchResults);
  endResetModel();
  setLoading(false);
}

void SearchModel::append(QList<SearchItem const*> const& searchResults)
{
  beginInsertRows(QModelIndex(), m_searchResults.size(), m_searchResults.size() + searchResults.size() - 1);
  m_searchResults.append(searchResults);
  endInsertRows();
  setLoading(false);
}

void SearchModel::search(Extractor* extractor)
{
  if (m_nextPage) {
    m_nextPage = nullptr;
    delete m_nextPage;
  }
  //m_contentFilters.clear();
  m_sortFilter.clear();
  setLoading(true);
  if (!m_searchTerm.isEmpty()) {
    extractor->search(this, m_searchTerm, m_contentFilters, m_sortFilter);
  }
  else {
    clear();
  }
}

void SearchModel::searchMore(Extractor* extractor)
{
  if (m_more) {
    if (!m_nextPage) {
      setLoading(true);
      extractor->search(this, m_searchTerm, m_contentFilters, m_sortFilter);
    }
    else {
      if (!(m_nextPage->id().isEmpty() && m_nextPage->ids().empty() && m_nextPage->url().isEmpty())) {
        setLoading(true);
        extractor->searchMore(this, m_searchTerm, m_contentFilters, m_sortFilter, m_nextPage);
      }
    }
  }
}

void SearchModel::clear()
{
  beginResetModel();
  for (SearchItem const* searchItem : m_searchResults) {
    delete searchItem;
  }
  m_searchResults.clear();
  endResetModel();
  setLoading(false);
}

PageRef* SearchModel::nextPage() const
{
  return m_nextPage;
}

void SearchModel::setNextPage(PageRef* nextPage)
{
  if (m_nextPage != nextPage) {
    if (m_nextPage && m_nextPage->parent() == this) {
      delete m_nextPage;
    }
    m_nextPage = nextPage;

    emit nextPageChanged();

    bool more = m_nextPage && !(m_nextPage->id().isEmpty() && m_nextPage->ids().isEmpty() && m_nextPage->url().isEmpty());
    setMore(more);
  }
}

bool SearchModel::loading() const
{
  return m_loading;
}

void SearchModel::setLoading(bool loading)
{
  if (m_loading != loading) {
    m_loading = loading;

    emit loadingChanged();
  }
}

bool SearchModel::more() const
{
  return m_more;
}

void SearchModel::setMore(bool more)
{
  if (m_more != more) {
    m_more = more;

    emit moreChanged();
  }
}

QString SearchModel::searchTerm() const
{
  return m_searchTerm;
}

void SearchModel::setSearchTerm(QString const& searchTerm)
{
  if (m_searchTerm != searchTerm) {
    m_searchTerm = searchTerm;

    emit searchTermChanged();
  }
}

QString SearchModel::contentFilter() const
{
  return m_contentFilters.isEmpty() ? QString() : m_contentFilters.first();
}

void SearchModel::setContentFilter(QString const& contentFilter)
{
  QString const& first = m_contentFilters.isEmpty() ? QString() : m_contentFilters.first();

  if (first != contentFilter) {
    m_contentFilters.clear();
    m_contentFilters.append(contentFilter);

    emit contentFilterChanged();
  }
}

QString SearchModel::sortFilter() const
{
  return m_sortFilter;
}

void SearchModel::setSortFilter(QString const& sortFilter)
{
  m_sortFilter = sortFilter;
}
