#include <QDebug>

#include "extractor.h"

#include "invoke.h"

Invoke::Invoke(Extractor* extractor, QString const methodName, QJsonDocument const* in)
  : QObject(dynamic_cast<QObject*>(extractor))
  , m_extractor(extractor)
  , m_methodName(methodName)
  , m_in(*in)
{
}

QJsonDocument Invoke::run()
{
  char const* result;
  QJsonDocument document;

  result = invoke(m_extractor->m_thread, m_methodName.toLatin1().data(), m_in.toJson().data());
  QJsonParseError error;
  document = QJsonDocument::fromJson(QByteArray(result), &error);
  if (document.isNull()) {
    qDebug() << "JSON Parsing error: " << error.errorString();
  }

  this->deleteLater();
  return document;
}
