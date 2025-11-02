#ifndef INVOKE_H
#define INVOKE_H

#include <QObject>
#include <QJsonDocument>

class Extractor;

class Invoke : public QObject {
  Q_OBJECT
public:
  explicit Invoke(Extractor* extractor, QString const methodName, QJsonDocument const* in);

  QJsonDocument run();

private:
  Extractor const* m_extractor;
  QString m_methodName;
  QJsonDocument m_in;

signals:
};

#endif // INVOKE_H
