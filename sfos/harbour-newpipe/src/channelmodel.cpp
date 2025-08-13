#include "pageref.h"
#include "extractor.h"
#include "channeltabinfo.h"
#include "listlinkhandler.h"

#include "channelmodel.h"

ChannelModel::ChannelModel(QObject *parent)
  : SearchModel(parent)
  , m_linkHandler(new ListLinkHandler(this))
  , m_channelTabInfo(new ChannelTabInfo(this))
{

}

void ChannelModel::search(Extractor* extractor)
{
  setLoading(true);
  extractor->getChannelTabInfo(m_channelTabInfo, m_linkHandler, this);
}

void ChannelModel::searchMore(Extractor* extractor)
{
  if (m_more) {
    if (m_nextPage && !(m_nextPage->id().isEmpty() && m_nextPage->ids().empty() && m_nextPage->url().isEmpty())) {
      setLoading(true);
      extractor->getMoreChannelItems(m_linkHandler, m_nextPage, this);
    }
  }
}

ListLinkHandler* ChannelModel::linkHandler() const
{
  return m_linkHandler;
}

void ChannelModel::setLinkHandler(ListLinkHandler* linkHandler)
{
  if (m_linkHandler != linkHandler) {
    m_linkHandler = linkHandler;

    emit linkHandlerChanged();
  }
}

ChannelTabInfo* ChannelModel::channelTabInfo() const
{
  return m_channelTabInfo;
}

void ChannelModel::setChannelTabInfo(ChannelTabInfo* channelTabInfo)
{
  if (m_channelTabInfo != channelTabInfo) {
    m_channelTabInfo = channelTabInfo;

    emit channelTabInfoChanged();
  }
}
