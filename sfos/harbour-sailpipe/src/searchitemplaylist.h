#ifndef SEARCHITEMPLAYLIST_H
#define SEARCHITEMPLAYLIST_H

#include "searchitem.h"
#include <QObject>

class SearchItemPlaylist : public SearchItem
{
  Q_OBJECT
public:
  explicit SearchItemPlaylist(QObject *parent = nullptr);
  explicit SearchItemPlaylist(QJsonObject const& json, QObject *parent = nullptr);

  QString uploaderName() const;
  bool uploaderVerified() const;
  qint64 streamCount() const;
  QString description() const;
  virtual QString getInfoRow() const override;

  void setUploaderName(QString const& uploaderName);
  void setUploaderVerified(bool uploaderVerified);
  void setStreamCount(qint64 streamCount);
  void setDescription(QString const& description);

  void parseJson(QJsonObject const& json) override;

private:
  QString m_uploaderName;
  bool m_uploaderVerified;
  qint64 m_streamCount;
  QString m_description;
};


#endif // SEARCHITEMPLAYLIST_H
