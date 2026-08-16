#ifndef PAGEREF_H
#define PAGEREF_H

#include <QObject>
#include <QMap>

class PageRef : public QObject
{
  Q_PROPERTY(QString url READ url CONSTANT)
  Q_PROPERTY(QString id READ id CONSTANT)
  Q_PROPERTY(QStringList ids READ ids CONSTANT)
  Q_PROPERTY(QMap<QString, QString> cookies READ cookies CONSTANT)
  Q_PROPERTY(QString body READ body CONSTANT)

  Q_OBJECT
public:
  explicit PageRef(QObject *parent = nullptr);
  explicit PageRef(QString const& url, QString const& id, QStringList const& ids, QMap<QString, QString> const& cookies, QString const& body, QObject *parent = nullptr);
  explicit PageRef(QJsonObject const& json, QObject *parent = nullptr);

  QString url() const;
  QString id() const;
  QStringList ids() const;
  QMap<QString, QString> cookies() const;
  QString body();
  QJsonObject toJson() const;

private:
  void setUrl(QString const& url);
  void setId(QString const& id);
  void setIds(QStringList const& ids);
  void setCookies(QMap<QString, QString> const& cookies);
  void setBody(QString const& body);
  void parseJson(QJsonObject const& json);

private:
  QString m_url;
  QString m_id;
  QStringList m_ids;
  QMap<QString, QString> m_cookies;
  QString m_body;
};

#endif // PAGEREF_H
