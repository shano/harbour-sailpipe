#ifndef UTILS_H
#define UTILS_H

#include <QDateTime>
#include <QObject>

class QQmlEngine;
class QJSEngine;

class Utils : public QObject
{
  Q_OBJECT
public:
  explicit Utils(QObject *parent = nullptr);
  static void instantiate(QObject* parent = nullptr);
  static Utils& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);
  Q_INVOKABLE static QString millisecondsToTime(quint32 milliseconds);
  Q_INVOKABLE static QString lengthToTimeString(quint64 length);
  Q_INVOKABLE static QDateTime epochToDateTime(qint64 epoch);
  Q_INVOKABLE QString getImageDir() const;
  Q_INVOKABLE QString getImageUrl(QString const &id) const;

private:
  static Utils* m_instance;
  QString m_imageDir;
};

#endif // UTILS_H
