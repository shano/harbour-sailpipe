#include <QQmlEngine>
#include <QDebug>

#include "mediajunction.h"

MediaJunction* MediaJunction::m_instance = nullptr;

MediaJunction::MediaJunction(QObject *parent)
  : QObject(parent)
  , m_controllable(false)
  , m_position(0)
  , m_playing(false)
  , m_title()
{
}

void MediaJunction::instantiate(QObject* parent) {
  if (m_instance == nullptr) {
    m_instance = new MediaJunction(parent);
  }
}

MediaJunction& MediaJunction::getInstance() {
  return *m_instance;
}

QObject* MediaJunction::provider(QQmlEngine* engine, QJSEngine* scriptEngine) {
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}


bool MediaJunction::controllable() const
{
  return m_controllable;
}

int MediaJunction::position() const
{
  return m_position;
}

bool MediaJunction::playing() const
{
  return m_playing;
}

QString MediaJunction::title() const
{
  return m_title;
}

void MediaJunction::setControllable(bool controllable)
{
  if (m_controllable != controllable) {
    m_controllable = controllable;
    emit controllableChanged();
  }
}

void MediaJunction::setPosition(int position)
{
  if (m_position != position) {
    m_position = position;
    emit positionChanged();
  }
}

void MediaJunction::setPlaying(bool playing)
{
  if (m_playing != playing) {
    m_playing = playing;
    emit playingChanged();
  }
}

void MediaJunction::setTitle(QString title)
{
  if (m_title!= title) {
    m_title = title;
    emit titleChanged();
  }
}

void MediaJunction::playPause()
{
  qDebug() << "MediaJunction: play/pause";
  if (m_controllable) {
    setPlaying(!m_playing);
  }
}

void MediaJunction::skipForwards()
{
  qDebug() << "MediaJunction: forwards";
}

void MediaJunction::skipBackwards()
{
  qDebug() << "MediaJunction: backwards";
}
