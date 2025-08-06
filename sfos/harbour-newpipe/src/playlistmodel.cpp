#include "playlistmodel.h"
#include "pageref.h"
#include "extractor.h"

PlaylistModel::PlaylistModel(QObject *parent)
  : SearchModel(parent)
  , m_url()
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

void PlaylistModel::setUrl(QString const& url)
{
  if (m_url != url) {
    m_url = url;

    emit urlChanged();
  }
}
