#include "extractor.h"

#include "filtermodel.h"

FilterModel::FilterModel(QObject *parent)
  : QAbstractListModel(parent)
  , m_loading(false)
  , m_filterData()
  , m_defaultFilter("all")
{
  m_roles[NameRole] = "name";
  m_roles[FilterRole] = "filter";
}

QHash<int, QByteArray> FilterModel::roleNames() const
{
  return m_roles;
}

int FilterModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent)
  return m_filterData.count();
}

QVariant FilterModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_filterData.count())) {
    QString const& data = m_filterData[index.row()];
    switch (role) {
      case NameRole:
        result = filterToName(data);
        break;
      case FilterRole:
        result = data;
        break;
    }
  }

  return result;
}

QString FilterModel::filterToName(QString const& filter)
{
  static const QMap<QString, QString> convert = {
    //% "All"
    {"all", qtTrId("newpipe-filter-name_all")},
    //% "Videos"
    {"videos", qtTrId("newpipe-filter-name_videos")},
    //% "Channels"
    {"channels", qtTrId("newpipe-filter-name_channels")},
    //% "Playlists"
    {"playlists", qtTrId("newpipe-filter-name_playlists")},
    //% "Music songs"
    {"music_songs", qtTrId("newpipe-filter-name_music-congs")},
    //% "Music videos"
    {"music_videos", qtTrId("newpipe-filter-name_music-videos")},
    //% "Music albums"
    {"music_albums", qtTrId("newpipe-filter-name_music-albums")},
    //% "Music playlists"
    {"music_playlists", qtTrId("newpipe-filter-name_music-playlists")}
  };
  QString result;

  result = convert.value(filter, QString());

  if (result.isEmpty() && !filter.isEmpty()) {
    result = filter.toLower();
    result = filter.at(0).toUpper() + filter.mid(1).toLower();
    result = result.replace("_", " ");
  }

  return result;
}

void FilterModel::replaceAll(QStringList const& filterResults)
{
  QString defaultFilter("");
  int oldCount = m_filterData.count();
  bool emitDefaultFilterChanged = false;

  if (!filterResults.isEmpty()) {
    defaultFilter = filterResults.first();
  }

  if (m_defaultFilter != defaultFilter) {
    m_defaultFilter = defaultFilter;

    emitDefaultFilterChanged = true;
  }

  beginResetModel();
  m_filterData = filterResults;
  endResetModel();
  m_loading = false;

  if (m_filterData.count() != oldCount) {
    emit countChanged();
  }

  if (emitDefaultFilterChanged) {
    emit defaultFilterChanged();
  }
}

void FilterModel::populate(Extractor* extractor)
{
  if (!m_loading) {
    m_loading = true;
    extractor->getAvailableContentFilter(this);
  }
}

QString FilterModel::defaultFilter() const
{
  return m_defaultFilter;
}

bool FilterModel::filterValid(QString const& filter) const
{
  bool valid = false;

  if (m_filterData.isEmpty()) {
    valid = filter.isEmpty();
  }
  else {
    valid = m_filterData.contains(filter);
  }

  return valid;
}

int FilterModel::count() const
{
  return m_filterData.count();
}
