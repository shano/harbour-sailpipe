#ifndef SPONSORBLOCKCLIENT_H
#define SPONSORBLOCKCLIENT_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVariantList>

class QQmlEngine;
class QJSEngine;

class SponsorBlockClient : public QObject
{
  Q_OBJECT

public:
  explicit SponsorBlockClient(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static SponsorBlockClient& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

public slots:
  // Looks up sponsor/self-promo/etc. segments for videoUrl via the public
  // SponsorBlock API and emits segmentsReady with a list of
  // {startMs, endMs} maps once the query completes. A video with no
  // reported segments (or a lookup failure) emits an empty list rather
  // than an error — this is a best-effort convenience feature, not a
  // critical path.
  void fetchSegments(QString const& videoUrl);

signals:
  void segmentsReady(QString const& videoUrl, QVariantList const& segments);

private:
  static SponsorBlockClient* m_instance;
  QNetworkAccessManager m_manager;
};

#endif // SPONSORBLOCKCLIENT_H
