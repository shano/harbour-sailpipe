#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include <QObject>
#include <QString>

class SearchItem : public QObject {
  Q_OBJECT
public:
  enum InfoType {
    None = -1,
    Stream,
    Playlist,
    Channel,
    Comment
  };
  Q_ENUM(InfoType)

  SearchItem(QObject *parent = nullptr);
  explicit SearchItem(QString const& name, QString const& thumbnail, QString const& url, QObject *parent = nullptr);
  explicit SearchItem(QJsonObject const& json, QObject *parent = nullptr);

  static SearchItem* createSearchItem(QJsonObject const& json, QObject *parent = nullptr);

  InfoType getInfoType() const;
  QString getName() const;
  QString getThumbnail() const;
  QString getUrl() const;
  virtual QString getInfoRow() const;
  virtual quint64 duration() const;

  void setInfoType(InfoType infoType);
  void setName(QString const& name);
  void setThumbnail(QString const& thumbnail);
  void setUrl(QString const& url);

  virtual void parseJson(QJsonObject const& json);

protected:
  InfoType m_infoType;
private:
  QString m_name;
  QString m_thumbnail;
  QString m_url;
};

#endif // SEARCHITEM_H
