#ifndef MEDIAJUNCTION_H
#define MEDIAJUNCTION_H

#include <QObject>

class MediaJunction : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool controllable READ controllable WRITE setControllable NOTIFY controllableChanged)
  Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
  Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
public:
  explicit MediaJunction(QObject *parent = nullptr);
  static void instantiate(QObject* parent = nullptr);
  static MediaJunction& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  bool controllable() const;
  int position() const;
  bool playing() const;
  QString title() const;

  void setControllable(bool controllable);
  void setPosition(int position);
  void setPlaying(bool playing);
  void setTitle(QString title);

public slots:
  void playPause();
  void skipForwards();
  void skipBackwards();

signals:
  void controllableChanged();
  void positionChanged();
  void playingChanged();
  void titleChanged();

private:
  static MediaJunction* m_instance;
  bool m_controllable;
  int m_position;
  bool m_playing;
  QString m_title;
};

#endif // MEDIAJUNCTION_H
