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
    {"all", qtTrId("sailpipe_filter_name-all")},
    //% "Videos"
    {"videos", qtTrId("sailpipe_filter_name-videos")},
    //% "Channels"
    {"channels", qtTrId("sailpipe_filter_name-channels")},
    //% "Playlists"
    {"playlists", qtTrId("sailpipe_filter_name-playlists")},
    //% "Music songs"
    {"music_songs", qtTrId("sailpipe_filter_name-music_songs")},
    //% "Music videos"
    {"music_videos", qtTrId("sailpipe_filter_name-music_videos")},
    //% "Music albums"
    {"music_albums", qtTrId("sailpipe_filter_name-music_albums")},
    //% "Music playlists"
    {"music_playlists", qtTrId("sailpipe_filter_name-music_playlists")},
    //% "Tracks"
    {"tracks", qtTrId("sailpipe_filter_name-tracks")},
    //% "Users"
    {"users", qtTrId("sailpipe_filter_name-users")},
    //% "Conferences"
    {"conferences", qtTrId("sailpipe_filter_name-conferences")},
    //% "Events"
    {"events", qtTrId("sailpipe_filter_name-events")},
    //% "Sepia videos"
    {"sepia_videos", qtTrId("sailpipe_filter_name-sepia_videos")}
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
