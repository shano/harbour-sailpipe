#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include "ytdlpmanager.h"
#include "sponsorblockclient.h"

namespace {

QString extractVideoId(QString const& videoUrl)
{
  static QRegularExpression const pattern(QStringLiteral("(?:v=|youtu\\.be/)([A-Za-z0-9_-]{11})"));
  QRegularExpressionMatch match = pattern.match(videoUrl);
  return match.hasMatch() ? match.captured(1) : QString();
}

} // namespace

SponsorBlockClient* SponsorBlockClient::m_instance = nullptr;

SponsorBlockClient::SponsorBlockClient(QObject *parent)
  : QObject(parent)
  , m_manager()
{
}

void SponsorBlockClient::instantiate(QObject* parent)
{
  if (m_instance == nullptr) {
    m_instance = new SponsorBlockClient(parent);
  }
}

SponsorBlockClient& SponsorBlockClient::getInstance()
{
  return *m_instance;
}

QObject* SponsorBlockClient::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

void SponsorBlockClient::fetchSegments(QString const& videoUrl)
{
  QString videoId = extractVideoId(videoUrl);
  if (videoId.isEmpty()) {
    emit segmentsReady(videoUrl, QVariantList());
    return;
  }

  QStringList categories = YtDlpManager::sponsorBlockCategories().split(QChar(','), QString::SkipEmptyParts);
  QJsonArray categoryArray;
  for (QString const& category : categories) {
    categoryArray.append(category);
  }

  QUrl url(QStringLiteral("https://sponsor.ajay.app/api/skipSegments"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("videoID"), videoId);
  query.addQueryItem(QStringLiteral("categories"), QString::fromUtf8(QJsonDocument(categoryArray).toJson(QJsonDocument::Compact)));
  url.setQuery(query);

  QNetworkReply* reply = m_manager.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished, this, [this, reply, videoUrl]() {
    QVariantList segments;

    // A 404 here just means "no segments reported for this video" — not
    // an error condition worth surfacing.
    if (reply->error() == QNetworkReply::NoError) {
      QJsonArray entries = QJsonDocument::fromJson(reply->readAll()).array();
      for (QJsonValue const& entry : entries) {
        QJsonArray segment = entry.toObject()[QStringLiteral("segment")].toArray();
        if (segment.size() == 2) {
          QVariantMap bounds;
          bounds[QStringLiteral("startMs")] = static_cast<qint64>(segment[0].toDouble() * 1000);
          bounds[QStringLiteral("endMs")] = static_cast<qint64>(segment[1].toDouble() * 1000);
          segments.append(bounds);
        }
      }
    }

    emit segmentsReady(videoUrl, segments);
    reply->deleteLater();
  });
}
