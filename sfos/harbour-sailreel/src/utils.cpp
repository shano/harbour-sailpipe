#include <sailfishapp.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <silicatheme.h>
#include <math.h>

#include "utils.h"

Utils* Utils::m_instance = nullptr;

Utils::Utils(QObject *parent) : QObject(parent)
{
  Silica::Theme *silicaTheme = Silica::Theme::instance();
  double pixelRatio = silicaTheme->pixelRatio();

  double quantised = std::min(std::max(std::ceil(pixelRatio * 4.0) / 4.0, 1.0), 2.0);
  QString dir = QString::number(quantised, 'f', 2);
  if (dir.length() == 4 && dir.at(3) == '0') {
    dir.truncate(3);
  }

  m_imageDir = SailfishApp::pathTo("qml/images/z" + dir).toString(QUrl::RemoveScheme) + "/";

  connect(silicaTheme, &Silica::Theme::colorSchemeChanged, this, &Utils::colorSchemeChanged);
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

  return QString::fromLatin1("%1:%2:%3").arg(hours).arg(minutes, 2, 'f', 0, '0').arg(seconds, 2, 'f', 0, '0');
}

QString Utils::lengthToTimeString(quint64 length)
{
  QString result;
  if (length > 0) {
    quint64 remaining = length;
    int hours = remaining / 3600;
    int minutes = (remaining / 60) % 60;
    int seconds = (remaining % 60);
    result = QString::fromLatin1("%1:%2:%3").arg(hours).arg(minutes, 2, 'f', 0, '0').arg(seconds, 2, 'f', 0, '0');
  }
  else {
    //% "??:??:??"
    result = qtTrId("sailpipe-utils_length_unknown");
  }

  return result;
}

QDateTime Utils::epochToDateTime(qint64 epoch)
{
  QDateTime dateTIme = QDateTime::fromMSecsSinceEpoch(epoch * 1000);

  return dateTIme;
}

QString Utils::getImageDir() const
{
  return m_imageDir;
}

QString Utils::getImageUrl(QString const &id) const
{
  Silica::Theme *silicaTheme = Silica::Theme::instance();
  QString suffix = silicaTheme->colorScheme() == Silica::Theme::DarkOnLight ? QString::fromLatin1("-light") : QString();
  return m_imageDir + id + suffix + QString::fromLatin1(".png");
}

void Utils::logDebug(QString const& message)
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(dataDir);
  QFile logFile(dataDir + QStringLiteral("/debug.log"));
  if (logFile.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream stream(&logFile);
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << ' ' << message << '\n';
  }
  qDebug() << message;
}

QString Utils::replayIcon() const
{
  return getImageUrl(QString::fromLatin1("icon-cover-replay"));
}

