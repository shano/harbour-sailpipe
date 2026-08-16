#include <QJsonObject>
#include <QJsonArray>

#include "pageref.h"

PageRef::PageRef(QObject *parent)
  : QObject(parent)
  , m_url()
  , m_id()
  , m_ids()
  , m_cookies()
  , m_body()
{
}

PageRef::PageRef(QString const& url, QString const& id, QStringList const& ids, QMap<QString, QString> const& cookies, QString const& body, QObject *parent)
  : QObject(parent)
  , m_url(url)
  , m_id(id)
  , m_ids(ids)
  , m_cookies(cookies)
  , m_body(body)
{
}

PageRef::PageRef(QJsonObject const& json, QObject *parent)
  : QObject(parent)
{
  parseJson(json);
}

QString PageRef::url() const
{
  return m_url;
}

QString PageRef::id() const
{
  return m_id;
}

QStringList PageRef::ids() const
{
  return m_ids;
}

QMap<QString, QString> PageRef::cookies() const
{
  return m_cookies;
}

QString PageRef::body()
{
  return m_body;
}

void PageRef::setUrl(QString const& url)
{
  m_url = url;
}

void PageRef::setId(QString const& id)
{
  m_id = id;
}

void PageRef::setIds(QStringList const& ids)
{
  m_ids = ids;
}

void PageRef::setCookies(QMap<QString, QString> const& cookies)
{
  m_cookies = cookies;
}

void PageRef::setBody(QString const& body)
{
  m_body = body;
}

void PageRef::parseJson(QJsonObject const& json)
{
  m_url = json["url"].toString();
  m_id = json["id"].toString();
  m_ids.clear();
  if (json["ids"].isArray()) {
    for (QJsonValue const& id : json["ids"].toArray()) {
      m_ids.append(id.toString());
    }
  }
  m_cookies.clear();
  if (json["cookies"].isObject()) {
    QJsonObject const cookies = json["cookies"].toObject();
    QJsonObject::const_iterator cookie = cookies.constBegin();
    while (cookie != cookies.constEnd()) {
      m_cookies.insert(cookie.key(), cookie.value().toString());
      ++cookie;
    }
  }
  m_body = QString();
  if (json["body"].isString()) {
    m_body = json["body"].toString();
  }
}

QJsonObject PageRef::toJson() const
{
  QJsonObject json;
  QJsonArray ids;
  QJsonObject cookies;

  json["url"] = m_url;
  json["id"] = m_id;

  for (QString const& id : m_ids) {
    ids.append(id);
  }
  json["ids"] = ids;

  QMap<QString, QString>::const_iterator cookie = m_cookies.constBegin();
  while (cookie != m_cookies.constEnd()) {
    cookies.insert(cookie.key(), cookie.value());
    ++cookie;
  }
  json["cookies"] = cookies;
  json["body"] = m_body.isNull() ? QJsonValue() : m_body;

  return json;
}
