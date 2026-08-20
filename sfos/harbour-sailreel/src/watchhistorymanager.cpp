#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include "watchhistorymanager.h"

#define WATCHED_THRESHOLD 0.9

WatchHistoryManager* WatchHistoryManager::m_instance = nullptr;

WatchHistoryManager::WatchHistoryManager(QObject *parent)
  : QObject(parent)
{
  load();
}

void WatchHistoryManager::instantiate(QObject* parent)
{
  if (m_instance == nullptr) {
    m_instance = new WatchHistoryManager(parent);
  }
}

WatchHistoryManager& WatchHistoryManager::getInstance()
{
  return *m_instance;
}

QObject* WatchHistoryManager::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

qint64 WatchHistoryManager::positionMs(QString const& url) const
{
  return m_entries.value(url).positionMs;
}

bool WatchHistoryManager::isWatched(QString const& url) const
{
  return m_entries.value(url).watched;
}

double WatchHistoryManager::fractionFor(QString const& url) const
{
  if (!m_entries.contains(url)) {
    return 0.0;
  }
  Entry const& entry = m_entries[url];
  if (entry.durationMs <= 0) {
    return 0.0;
  }
  return qBound(0.0, static_cast<double>(entry.positionMs) / entry.durationMs, 1.0);
}

void WatchHistoryManager::setProgress(QString const& url, qint64 positionMs, qint64 durationMs)
{
  if (url.isEmpty() || (durationMs <= 0)) {
    return;
  }

  Entry entry;
  entry.positionMs = positionMs;
  entry.durationMs = durationMs;
  entry.watched = (static_cast<double>(positionMs) / durationMs) >= WATCHED_THRESHOLD;
  m_entries[url] = entry;

  save();
}

QString WatchHistoryManager::storagePath()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return dataDir + QStringLiteral("/watchhistory.json");
}

void WatchHistoryManager::load()
{
  QFile file(storagePath());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
  m_entries.clear();
  for (QJsonValue const& value : array) {
    QJsonObject object = value.toObject();
    QString url = object[QStringLiteral("url")].toString();
    if (url.isEmpty()) {
      continue;
    }
    Entry entry;
    entry.positionMs = static_cast<qint64>(object[QStringLiteral("positionMs")].toDouble());
    entry.durationMs = static_cast<qint64>(object[QStringLiteral("durationMs")].toDouble());
    entry.watched = object[QStringLiteral("watched")].toBool();
    m_entries[url] = entry;
  }
}

void WatchHistoryManager::save() const
{
  QString path = storagePath();
  QDir().mkpath(QFileInfo(path).absolutePath());

  QJsonArray array;
  for (auto it = m_entries.constBegin(); it != m_entries.constEnd(); ++it) {
    QJsonObject object;
    object[QStringLiteral("url")] = it.key();
    object[QStringLiteral("positionMs")] = it.value().positionMs;
    object[QStringLiteral("durationMs")] = it.value().durationMs;
    object[QStringLiteral("watched")] = it.value().watched;
    array.append(object);
  }

  QFile file(path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
  }
}
