#ifndef WATCHHISTORYMANAGER_H
#define WATCHHISTORYMANAGER_H

#include <QHash>
#include <QObject>
#include <QString>

class QQmlEngine;
class QJSEngine;

class WatchHistoryManager : public QObject
{
  Q_OBJECT

public:
  explicit WatchHistoryManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static WatchHistoryManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

public slots:
  qint64 positionMs(QString const& url) const;
  bool isWatched(QString const& url) const;
  double fractionFor(QString const& url) const;
  void setProgress(QString const& url, qint64 positionMs, qint64 durationMs);

private:
  struct Entry {
    qint64 positionMs;
    qint64 durationMs;
    bool watched;
  };

  void load();
  void save() const;
  static QString storagePath();

  static WatchHistoryManager* m_instance;
  QHash<QString, Entry> m_entries;
};

#endif // WATCHHISTORYMANAGER_H
