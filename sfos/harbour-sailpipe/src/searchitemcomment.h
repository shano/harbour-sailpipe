#ifndef SEARCHITEMCOMMENT_H
#define SEARCHITEMCOMMENT_H

#include "searchitem.h"
#include <QObject>

class SearchItemComment : public SearchItem
{
  Q_OBJECT
public:
  explicit SearchItemComment(QObject *parent = nullptr);
  explicit SearchItemComment(QJsonObject const& json, QObject *parent = nullptr);

  QString description() const;

  void setDescription(QString const& uploaderName);

  void parseJson(QJsonObject const& json) override;

private:
  QString m_description;
};


#endif // SEARCHITEMCOMMENT_H
