#include "pageref.h"
#include "extractor.h"

#include "commentmodel.h"

CommentModel::CommentModel(QObject *parent)
  : QAbstractListModel(parent)
  , m_nextPage()
  , m_loading(false)
  , m_more(true)
{
  m_roles[CommentTextRole] = "commentText";
  m_roles[UploaderNameRole] = "uploaderName";
  m_roles[UploaderAvatarRole] = "uploaderAvatar";
  m_roles[ReplyCountRole] = "replyCount";
  m_roles[PageRole] = "page";
}

QHash<int, QByteArray> CommentModel::roleNames() const
{
  return m_roles;
}

int CommentModel::rowCount(const QModelIndex & parent) const
{
  Q_UNUSED( parent)
  return m_commentResults.count();
}

QVariant CommentModel::data(const QModelIndex & index, int role) const
{
  QVariant result = QVariant();

  if ((index.row() >= 0) && (index.row() < m_commentResults.count())) {
    CommentItem const* commentResult = m_commentResults[index.row()];
    if (role == CommentTextRole)
      result = commentResult->getCommentText();
    else if (role == UploaderNameRole)
      result = commentResult->getUploaderName();
    else if (role == UploaderAvatarRole)
      result = commentResult->getUploaderAvatar();
    else if (role == ReplyCountRole)
      result = commentResult->getReplyCount();
    else if (role == PageRole)
      result = QVariant::fromValue(commentResult->getPage());
  }

  return result;
}

void CommentModel::replaceAll(QList<CommentItem const*> const& commentResults)
{
  beginResetModel();
  for (CommentItem const* commentItem : m_commentResults) {
    delete commentItem;
  }
  m_commentResults.clear();
  m_commentResults.append(commentResults);
  endResetModel();
  setLoading(false);
}

void CommentModel::append(QList<CommentItem const*> const& commentResults)
{
  beginInsertRows(QModelIndex(), m_commentResults.size(), m_commentResults.size() + commentResults.size() - 1);
  m_commentResults.append(commentResults);
  endInsertRows();
  setLoading(false);
}

void CommentModel::loadComments(Extractor* extractor, QString const& url)
{
  if (m_more) {
    if (!m_nextPage) {
      setLoading(true);
      extractor->getComments(this, url);
    }
    else {
      if (!(m_nextPage->id().isEmpty() && m_nextPage->url().isEmpty())) {
        if (m_commentResults.isEmpty()) {
          setLoading(true);
          extractor->getMoreComments(this, url, m_nextPage);
        }
        else {
          setLoading(true);
          extractor->appendMoreComments(this, url, m_nextPage);
        }
      }
    }
  }
}

PageRef* CommentModel::nextPage() const
{
  return m_nextPage;
}

void CommentModel::setNextPage(PageRef* nextPage)
{
  if (m_nextPage != nextPage) {
    if (m_nextPage && m_nextPage->parent() == this) {
      delete m_nextPage;
    }
    m_nextPage = nextPage;

    emit nextPageChanged();

    bool more = m_nextPage && !(m_nextPage->id().isEmpty() && m_nextPage->url().isEmpty());
    setMore(more);
  }
}

bool CommentModel::loading() const
{
  return m_loading;
}

void CommentModel::setLoading(bool loading)
{
  if (m_loading != loading) {
    m_loading = loading;

    emit loadingChanged();
  }
}

bool CommentModel::more() const
{
  return m_more;
}

void CommentModel::setMore(bool more)
{
  if (m_more != more) {
    m_more = more;
    emit moreChanged();
  }
}
