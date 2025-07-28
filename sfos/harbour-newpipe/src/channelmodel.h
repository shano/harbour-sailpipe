#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include "searchmodel.h"
#include <QObject>

class ListLinkHandler;
class ChannelTabInfo;

class ChannelModel : public SearchModel
{
  Q_PROPERTY(ListLinkHandler* linkHandler READ linkHandler WRITE setLinkHandler NOTIFY linkHandlerChanged)
  Q_PROPERTY(ChannelTabInfo* channelTabInfo READ channelTabInfo WRITE setChannelTabInfo NOTIFY channelTabInfoChanged)

  Q_OBJECT
public:
  explicit ChannelModel(QObject *parent = nullptr);

  ListLinkHandler* linkHandler() const;
  void setLinkHandler(ListLinkHandler* linkHandler);

  ChannelTabInfo* channelTabInfo() const;
  void setChannelTabInfo(ChannelTabInfo* channelTabInfo);

signals:
  void linkHandlerChanged();
  void channelTabInfoChanged();

public slots:
  virtual void search(Extractor* extractor) override;
  virtual void searchMore(Extractor* extractor) override;

private:
  ListLinkHandler* m_linkHandler;
  ChannelTabInfo* m_channelTabInfo;
};

Q_DECLARE_METATYPE(ChannelModel*)

#endif // CHANNELMODEL_H
