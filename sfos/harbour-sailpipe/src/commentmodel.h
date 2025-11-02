#ifndef COMMENTMODEL_H
#define COMMENTMODEL_H

#include <QAbstractListModel>

#include "commentitem.h"

class Extractor;

class CommentModel : public QAbstractListModel
{
  Q_PROPERTY(PageRef* nextPage READ nextPage WRITE setNextPage NOTIFY nextPageChanged)
  Q_PROPERTY(bool loading READ loading WRITE setLoading NOTIFY loadingChanged)
  Q_PROPERTY(bool more READ more WRITE setMore NOTIFY moreChanged)

  Q_OBJECT
public:
  enum CommentRoles {
    CommentTextRole = Qt::UserRole + 1,
    UploaderNameRole,
    UploaderAvatarRole,
    ReplyCountRole,
    PageRole,
  };

  explicit CommentModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const;

  int rowCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

  void replaceAll(QList<CommentItem const*> const& commentResults);

  void append(QList<CommentItem const*> const& commentResults);

public slots:
  void loadComments(Extractor* extractor, QString const& url);

  PageRef* nextPage() const;
  void setNextPage(PageRef* nextPage);

  bool loading() const;
  void setLoading(bool loading);

  bool more() const;
  void setMore(bool more);

signals:
  void nextPageChanged();
  void loadingChanged();
  void moreChanged();

private:
  QHash<int, QByteArray> m_roles;
  QList<CommentItem const*> m_commentResults;
  PageRef* m_nextPage;
  bool m_loading;
  bool m_more;
};

#endif // COMMENTMODEL_H
