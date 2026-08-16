#include <QQmlEngine>
#include <QDebug>
#include <mprisplayer.h>
#include <mprismetadata.h>

#include "mediajunction.h"

using namespace Amber;

MediaJunction* MediaJunction::m_instance = nullptr;

MediaJunction::MediaJunction(QObject *parent)
  : QObject(parent)
  , m_controllable(false)
  , m_position(0)
  , m_duration(0)
  , m_playing(false)
  , m_title()
  , m_creator()
  , m_thumbnail()
  , m_mpris(new MprisPlayer(this))
{
  connect(m_mpris, &MprisPlayer::playPauseRequested, this, &MediaJunction::playPauseRequested);
  connect(m_mpris, &MprisPlayer::nextRequested, this, &MediaJunction::skipForwardsRequested);
  connect(m_mpris, &MprisPlayer::previousRequested, this, &MediaJunction::skipBackwardsRequested);

  m_mpris->setServiceName(m_controllable ? QString::fromLatin1("harbour-sailreel") : QString());

  //% "SailReel"
  m_mpris->setIdentity(qtTrId("sailpipe-mpris_identity"));
  m_mpris->setDesktopEntry("harbour-sailreel");
  m_mpris->setCanControl(m_controllable);
  m_mpris->setCanPlay(m_controllable);
  m_mpris->setCanPause(m_controllable);
  m_mpris->setCanSeek(m_controllable);
  m_mpris->setCanGoNext(m_controllable);
  m_mpris->setCanGoPrevious(m_controllable);
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

int MediaJunction::duration() const
{
  return m_duration;
}

bool MediaJunction::playing() const
{
  return m_playing;
}

QString MediaJunction::title() const
{
  return m_title;
}

QString MediaJunction::creator() const
{
  return m_creator;
}

QString MediaJunction::thumbnail() const
{
  return m_thumbnail;
}

void MediaJunction::setControllable(bool controllable)
{
  if (m_controllable != controllable) {
    m_controllable = controllable;
    emit controllableChanged();
    m_mpris->setServiceName(m_controllable ? QString::fromLatin1("harbour-sailreel") : QString());
    m_mpris->setCanControl(m_controllable);
    m_mpris->setCanPlay(m_controllable);
    m_mpris->setCanPause(m_controllable);
    m_mpris->setCanSeek(m_controllable);
    m_mpris->setCanGoNext(m_controllable);
    m_mpris->setCanGoPrevious(m_controllable);
    qDebug() << "MediaJunction: controllable: " << m_controllable;
  }
}

void MediaJunction::setPosition(int position)
{
  if (m_position != position) {
    m_position = position;
    emit positionChanged();
    m_mpris->setPosition(m_position / 1000);
  }
}

void MediaJunction::setDuration(int duration)
{
  if (m_duration != duration) {
    m_duration = duration;
    emit durationChanged();
    m_mpris->metaData()->setDuration(m_duration / 1000);
    qDebug() << "MediaJunction: duration: " << m_duration;
  }
}

void MediaJunction::setPlaying(bool playing)
{
  if (m_playing != playing) {
    m_playing = playing;
    emit playingChanged();
    m_mpris->setPlaybackStatus(m_playing ? Amber::Mpris::PlaybackStatus::Playing : Amber::Mpris::PlaybackStatus::Paused);
    qDebug() << "MediaJunction: playing: " << m_playing;
  }
}

void MediaJunction::setTitle(QString title)
{
  if (m_title!= title) {
    m_title = title;
    emit titleChanged();
    m_mpris->metaData()->setTitle(m_title);
    qDebug() << "MediaJunction: title: " << m_title;
  }
}

void MediaJunction::setCreator(QString creator)
{
  if (m_creator!= creator) {
    m_creator = creator;
    emit creatorChanged();
    m_mpris->metaData()->setContributingArtist(m_creator);
    qDebug() << "MediaJunction: creator: " << m_creator;
  }
}

void MediaJunction::setThumbnail(QString thumbnail)
{
  if (m_thumbnail!= thumbnail) {
    m_thumbnail = thumbnail;
    emit thumbnailChanged();
    m_mpris->metaData()->setArtUrl(m_thumbnail);
    qDebug() << "MediaJunction: thumbnail: " << m_thumbnail;
  }
}
