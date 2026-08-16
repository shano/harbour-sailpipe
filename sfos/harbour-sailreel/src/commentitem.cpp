#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <cmath>

#include "extractor.h"
#include "pageref.h"

#include "commentitem.h"

CommentItem::CommentItem(QObject *parent)
  : QObject(parent)
  , m_commentText()
  , m_uploaderName()
  , m_uploaderAvatar()
  , m_replyCount(0)
  , m_page()
{

}

CommentItem::CommentItem(QString const& commentText, QString const& uploaderName, QString const& uploaderAvatar, qint64 replyCount, PageRef* page, QObject *parent)
  : QObject(parent)
  , m_commentText(commentText)
  , m_uploaderName(uploaderName)
  , m_uploaderAvatar(uploaderAvatar)
  , m_replyCount(replyCount)
  , m_page(page)
{
}

CommentItem::CommentItem(QJsonObject const& json, QObject *parent )
  : CommentItem(parent)
{
  parseJson(json);
}

QString CommentItem::getCommentText() const
{
  return m_commentText;
}

QString CommentItem::getUploaderName() const
{
  return m_uploaderName;
}

QString CommentItem::getUploaderAvatar() const
{
  return m_uploaderAvatar;
}

qint64 CommentItem::getReplyCount() const
{
  return m_replyCount;
}

PageRef* CommentItem::getPage() const
{
  return m_page;
}

void CommentItem::setCommentText(QString const& commentText)
{
  m_commentText = commentText;
}

void CommentItem::setUploaderName(QString const& uploaderName)
{
  m_uploaderName = uploaderName;
}

void CommentItem::setUploaderAvatar(QString const& uploaderAvatar)
{
  m_uploaderAvatar = uploaderAvatar;
}

void CommentItem::setReplyCount(qint64 replyCount)
{
  m_replyCount = replyCount;
}

void CommentItem::setPage(PageRef* page)
{
  if (m_page) {
    delete m_page;
  }
  m_page = page;
}

void CommentItem::parseJson(QJsonObject const& json)
{
  const int idealWidth = 64;
  const int idealHeight = 64;
  QString uploaderAvatarUrl;
  QJsonArray uploaderAvatars = json["uploaderAvatars"].toArray();
  float minRmse = -1.0;
  for (QJsonValue const& uploaderAvatar : uploaderAvatars) {
    QJsonObject details = uploaderAvatar.toObject();
    int width = details["width"].toInt();
    int height = details["height"].toInt();
    float rmse = std::pow(std::pow(idealWidth - width, 2) + std::pow(idealHeight - height, 2), 0.5f);
    if ((minRmse < 0) || (rmse < minRmse)) {
      uploaderAvatarUrl = details["url"].toString();
      minRmse = rmse;
    }
  }
  m_commentText = json["commentText"].toObject()["content"].toString();
  m_uploaderName = json["uploaderName"].toString();
  m_uploaderAvatar = uploaderAvatarUrl;
  m_replyCount = json["replyCount"].toInt(0);
  if (m_page) {
    delete m_page;
  }
  m_page = new PageRef(json["replies"].toObject(), this);
}
