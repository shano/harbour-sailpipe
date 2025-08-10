#include <sailfishapp.h>

#include <QTextStream>

#include "utils.h"

Utils* Utils::m_instance = nullptr;

Utils::Utils(QObject *parent) : QObject(parent)
{

}

void Utils::instantiate(QObject* parent) {
  if (m_instance == nullptr) {
    m_instance = new Utils(parent);
  }
}

Utils& Utils::getInstance() {
  return *m_instance;
}

QObject* Utils::provider(QQmlEngine* engine, QJSEngine* scriptEngine) {
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

QString Utils::millisecondsToTime(quint32 milliseconds)
{
  QString result;
  int remaining = milliseconds / 1000;
  int hours = remaining / 3600;
  int minutes = (remaining / 60) % 60;
  int seconds = (remaining % 60);

  return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 'f', 0, '0').arg(seconds, 2, 'f', 0, '0');
}

QString Utils::lengthToTimeString(quint64 length)
{
  QString result;
  if (length > 0) {
    quint64 remaining = length;
    int hours = remaining / 3600;
    int minutes = (remaining / 60) % 60;
    int seconds = (remaining % 60);
    result = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 'f', 0, '0').arg(seconds, 2, 'f', 0, '0');
  }
  else {
    //% "??:??:??"
    result = qtTrId("newpipe-utils_length_unknown");
  }

  return result;
}

QDateTime Utils::epochToDateTime(qint64 epoch)
{
  QDateTime dateTIme = QDateTime::fromMSecsSinceEpoch(epoch * 1000);

  return dateTIme;
}

