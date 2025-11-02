#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QObject>

#include "searchmodel.h"

class PlaylistModel : public SearchModel
{
  Q_OBJECT
  Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
  Q_PROPERTY(DescType descriptionType READ descriptionType WRITE setDescriptionType NOTIFY descriptionTypeChanged)
  Q_PROPERTY(qint64 streamCount READ streamCount WRITE setStreamCount NOTIFY streamCountChanged)
  Q_PROPERTY(QString uploaderName READ uploaderName WRITE setUploaderName NOTIFY uploaderNameChanged)
  Q_PROPERTY(QString uploaderAvatar READ uploaderAvatar WRITE setUploaderName NOTIFY uploaderNameChanged)
  Q_PROPERTY(quint64 duration READ duration NOTIFY durationChanged)

public:
  enum DescType {
    HTML = 1,
    Markdown= 2,
    PlainText = 3,
  };
  Q_ENUM(DescType)

  explicit PlaylistModel(QObject *parent = nullptr);

  QString url() const;
  QString description() const;
  DescType descriptionType() const;
  qint64 streamCount() const;
  QString uploaderName() const;
  QString uploaderAvatar() const;
  quint64 duration() const;

  void setUrl(QString const& url);
  void setDescription(QString const& description);
  void setDescriptionType(DescType descriptionType);
  void setStreamCount(qint64 streamCount);
  void setUploaderName(QString const& uploaderName);
  void setUploaderAvatar(QString const& uploaderAvatar);

  void parseJson(QJsonObject const& json);
  void calculateDuration();

private:
  typedef void (PlaylistModel::*PlaylistModelSignal)();

  QList<PlaylistModelSignal> parseJsonChanges(QJsonObject const& json);

public slots:
  virtual void search(Extractor* extractor) override;
  virtual void searchMore(Extractor* extractor) override;

signals:
  void urlChanged();
  void descriptionChanged();
  void descriptionTypeChanged();
  void streamCountChanged();
  void uploaderNameChanged();
  void uploaderAvatarChanged();
  void durationChanged();

private:
  QString m_url;
  QString m_description;
  DescType m_descriptionType;
  qint64 m_streamCount;
  QString m_uploaderName;
  QString m_uploaderAvatar;
  quint64 m_duration;
};

#endif // PLAYLISTMODEL_H
