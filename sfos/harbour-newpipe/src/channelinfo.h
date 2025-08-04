#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include <QObject>

class ListLinkHandler;

class ChannelInfo : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
  Q_PROPERTY(int subscriberCount READ subscriberCount WRITE setSubscriberCount NOTIFY subscriberCountChanged)
  Q_PROPERTY(bool verified READ verified WRITE setVerified NOTIFY verifiedChanged)
  Q_PROPERTY(QString tags READ tags WRITE setTags NOTIFY tagsChanged)
public:
  explicit ChannelInfo(QObject *parent = nullptr);
  explicit ChannelInfo(QJsonObject const& json, QObject *parent = nullptr);

public slots:
  QString id() const;
  QString name() const;
  QString url() const;
  QString description() const;
  QList<ListLinkHandler*> tabs();
  qint64 subscriberCount() const;
  bool verified() const;
  QString tags() const;

  void setId(QString const& id);
  void setName(QString const& name);
  void setUrl(QString const& url);
  void setDescription(QString const& description);
  void setTabs(QList<ListLinkHandler*> const& tabs);
  void setSubscriberCount(qint64 subscriberCount);
  void setVerified(bool verified);
  void setTags(QString& tags);

  void parseJson(QJsonObject const& json);

signals:
  void descriptionChanged();
  void subscriberCountChanged();
  void verifiedChanged();
  void tagsChanged();

private:
  QString m_id;
  QString m_name;
  QString m_url;
  QString m_description;
  qint64 m_subscriberCount;
  bool m_verified;
  QString m_tags;
  QList<ListLinkHandler*> m_tabs;
};

#endif // CHANNELINFO_H
