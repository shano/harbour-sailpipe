#ifndef SEARCHITEMSTREAM_H
#define SEARCHITEMSTREAM_H

#include "searchitem.h"
#include <QObject>

class SearchItemStream : public SearchItem
{
  Q_OBJECT
public:
  explicit SearchItemStream(QObject *parent = nullptr);
  explicit SearchItemStream(QJsonObject const& json, QObject *parent = nullptr);

  QString uploaderName() const;
  quint64 uploadDate() const;
  QString textualUploadDate() const;
  virtual quint64 duration() const override;
  virtual QString getInfoRow() const override;

  void setUploaderName(QString const& uploaderName);
  void setUploadDate(quint64 uploadDate);
  void setTextualUploadDate(QString const& textualUploadDate);
  void setDuration(quint64 duration);

  virtual void parseJson(QJsonObject const& json) override;

private:
  QString m_uploaderName;
  quint64 m_uploadDate;
  QString m_textualUploadDate;
  quint64 m_duration;
};

#endif // SEARCHITEMSTREAM_H
