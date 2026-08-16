#ifndef SEARCHITEMCHANNEL_H
#define SEARCHITEMCHANNEL_H

#include "searchitem.h"
#include <QObject>

class SearchItemChannel : public SearchItem
{
  Q_OBJECT
public:
  explicit SearchItemChannel(QObject *parent = nullptr);
  explicit SearchItemChannel(QJsonObject const& json, QObject *parent = nullptr);

  QString description() const;
  qint64 subscriberCount() const;
  qint64 streamCount() const;
  bool verified() const;
  virtual QString getInfoRow() const override;

  void setDescription(QString const& description);
  void setSubscriberCount(qint64 subscriberCount);
  void setStreamCount(qint64 streamCount);
  void setVerified(bool verified);

  void parseJson(QJsonObject const& json) override;

private:
  QString m_description;
  qint64 m_subscriberCount;
  qint64 m_streamCount;
  bool m_verified;
};

#endif // SEARCHITEMCHANNEL_H
