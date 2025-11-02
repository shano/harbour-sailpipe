#include <QJsonObject>

#include "searchitemcomment.h"

SearchItemComment::SearchItemComment(QObject *parent)
  : SearchItem(parent)
  , m_description()
{
  m_infoType = Comment;
}

SearchItemComment::SearchItemComment(QJsonObject const& json, QObject *parent)
  : SearchItem(parent)
{
  m_infoType = Stream;
  parseJson(json);
}

QString SearchItemComment::description() const
{
  return m_description;
}

void SearchItemComment::setDescription(QString const& description)
{
  m_description = description;
}

void SearchItemComment::parseJson(QJsonObject const& json)
{
  SearchItem::parseJson(json);

  m_description = json["description"].toString();
}
