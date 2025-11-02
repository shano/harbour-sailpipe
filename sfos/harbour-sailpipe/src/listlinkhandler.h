#ifndef LISTLINKHANDLER_H
#define LISTLINKHANDLER_H

#include <QJsonObject>
#include <QObject>

class ListLinkHandler : public QObject
{
  Q_OBJECT
public:
  explicit ListLinkHandler(QObject *parent = nullptr);
  explicit ListLinkHandler(QString const& originalUrl, QString const& url, QString const& id, QStringList const& contentFilters, QString const& sortFilter, QObject *parent = nullptr);
  explicit ListLinkHandler(QJsonObject const& json, QObject *parent = nullptr);

public slots:
  QString originalUrl() const;
  QString url() const;
  QString id() const;
  QStringList contentFilters() const;
  QString sortFilter() const;

  void setOriginalUrl(QString const& originalUrl);
  void setUrl(QString const& url);
  void setId(QString const& id);
  void setContentFilters(QStringList const& contentFilters);
  void setSortFilter(QString const& sortFilter);

  QJsonObject toJson() const;
  void parseJson(QJsonObject const& json);

signals:

private:
  QString m_originalUrl;
  QString m_url;
  QString m_id;
  QStringList m_contentFilters;
  QString m_sortFilter;
};

Q_DECLARE_METATYPE(ListLinkHandler*)

#endif // LISTLINKHANDLER_H
