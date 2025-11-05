#ifndef MEDIAJUNCTION_H
#define MEDIAJUNCTION_H

#include <mprisplayer.h>

#include <QObject>

class MediaJunction : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool controllable READ controllable WRITE setControllable NOTIFY controllableChanged)
  Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
  Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
  Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
  Q_PROPERTY(QString thumbnail READ thumbnail WRITE setThumbnail NOTIFY thumbnailChanged)
public:
  explicit MediaJunction(QObject *parent = nullptr);
  static void instantiate(QObject* parent = nullptr);
  static MediaJunction& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  bool controllable() const;
  int position() const;
  int duration() const;
  bool playing() const;
  QString title() const;
  QString thumbnail() const;

  void setControllable(bool controllable);
  void setPosition(int position);
  void setDuration(int duration);
  void setPlaying(bool playing);
  void setTitle(QString title);
  void setThumbnail(QString thumbnail);

signals:
  void controllableChanged();
  void positionChanged();
  void durationChanged();
  void playingChanged();
  void titleChanged();
  void thumbnailChanged();

  void playPauseRequested();
  void positionRequested();
  void skipForwardsRequested();
  void skipBackwardsRequested();

private:
  static MediaJunction* m_instance;
  bool m_controllable;
  int m_position;
  int m_duration;
  bool m_playing;
  QString m_title;
  QString m_thumbnail;
  Amber::MprisPlayer* m_mpris;
};

#endif // MEDIAJUNCTION_H
