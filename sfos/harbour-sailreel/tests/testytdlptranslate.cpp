#include <QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>

#include "../src/ytdlptranslate.h"

class TestYtDlpTranslate : public QObject
{
  Q_OBJECT

private:
  QJsonObject loadFixture(QString const& name)
  {
    QFile file(QString("%1/testdata/%2").arg(TESTDATA_DIR, name));
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object();
  }

private slots:
  void streamItem_mapsTitleAndUrl();
  void streamItem_fallsBackToWatchUrlWhenWebpageUrlMissing();
  void channelItem_mapsSubscriberCountAndUrl();
  void searchResults_classifiesChannelEntryAsChannelNotStream();
  void searchResults_setsNextPageWhenMoreAvailable();
  void searchResults_omitsNextPageWhenExhausted();
  void mediaInfo_mapsContentFromResolvedUrl();
  void commentItem_mapsAuthorAndText();
};

void TestYtDlpTranslate::streamItem_mapsTitleAndUrl()
{
  QJsonObject entry = loadFixture("ytdlp_search_entry.json");

  QJsonObject result = YtDlpTranslate::streamItem(entry);

  QCOMPARE(result["infoType"].toString(), QStringLiteral("STREAM"));
  QVERIFY(!result["name"].toString().isEmpty());
  QVERIFY(result["url"].toString().startsWith(QStringLiteral("https://www.youtube.com/watch?v=")));
}

void TestYtDlpTranslate::streamItem_fallsBackToWatchUrlWhenWebpageUrlMissing()
{
  QJsonObject entry;
  entry["id"] = "abc123";
  entry["title"] = "Test video";

  QJsonObject result = YtDlpTranslate::streamItem(entry);

  QCOMPARE(result["url"].toString(), QStringLiteral("https://www.youtube.com/watch?v=abc123"));
}

void TestYtDlpTranslate::channelItem_mapsSubscriberCountAndUrl()
{
  QJsonObject entry = loadFixture("ytdlp_channel_search_entry.json");

  QJsonObject result = YtDlpTranslate::channelItem(entry);

  QCOMPARE(result["infoType"].toString(), QStringLiteral("CHANNEL"));
  QCOMPARE(result["name"].toString(), QStringLiteral("The Spiffing Brit"));
  QCOMPARE(result["url"].toString(), QStringLiteral("https://www.youtube.com/channel/UCRHXUZ0BxbkU2MYZgsuFgkQ"));
  QCOMPARE(result["subscriberCount"].toInt(), 4480000);
  QVERIFY(result["verified"].toBool());
}

void TestYtDlpTranslate::searchResults_classifiesChannelEntryAsChannelNotStream()
{
  QJsonObject channelEntry = loadFixture("ytdlp_channel_search_entry.json");
  QJsonObject videoEntry = loadFixture("ytdlp_search_entry.json");

  QJsonArray entries;
  entries.append(channelEntry);
  entries.append(videoEntry);

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, 10, 10);

  QJsonArray items = result["relatedItems"].toArray();
  QCOMPARE(items.size(), 2);
  QCOMPARE(items[0].toObject()["infoType"].toString(), QStringLiteral("CHANNEL"));
  QCOMPARE(items[1].toObject()["infoType"].toString(), QStringLiteral("STREAM"));
}

void TestYtDlpTranslate::searchResults_setsNextPageWhenMoreAvailable()
{
  QJsonArray entries;
  for (int i = 0; i < 5; ++i) {
    QJsonObject entry;
    entry["id"] = QString("video%1").arg(i);
    entry["title"] = QString("Video %1").arg(i);
    entries.append(entry);
  }

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, 3, 5);

  QCOMPARE(result["relatedItems"].toArray().size(), 3);
  QVERIFY(result["nextPage"].toObject().contains("id"));
  QCOMPARE(result["nextPage"].toObject()["id"].toString(), QStringLiteral("3"));
}

void TestYtDlpTranslate::searchResults_omitsNextPageWhenExhausted()
{
  QJsonArray entries;
  QJsonObject entry;
  entry["id"] = "onlyvideo";
  entry["title"] = "Only Video";
  entries.append(entry);

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, 10, 10);

  QCOMPARE(result["relatedItems"].toArray().size(), 1);
  QVERIFY(!result["nextPage"].toObject().contains("id"));
}

void TestYtDlpTranslate::mediaInfo_mapsContentFromResolvedUrl()
{
  QJsonObject info = loadFixture("ytdlp_video_info.json");

  QJsonObject result = YtDlpTranslate::mediaInfo(info);

  QVERIFY(!result["name"].toString().isEmpty());
  QVERIFY(!result["content"].toString().isEmpty());
  QCOMPARE(result["uploaderUrl"].toString(), QStringLiteral("https://www.youtube.com/channel/UCuAXFkgsw1L7xaCfnd5JJOw"));
  QCOMPARE(result["description"].toObject()["type"].toInt(), 3);
}

void TestYtDlpTranslate::commentItem_mapsAuthorAndText()
{
  QJsonObject comment = loadFixture("ytdlp_comment.json");

  QJsonObject result = YtDlpTranslate::commentItem(comment);

  QCOMPARE(result["uploaderName"].toString(), QStringLiteral("Someone"));
  QCOMPARE(result["commentText"].toObject()["content"].toString(),
    QStringLiteral("Great video, thanks for sharing!"));
  QCOMPARE(result["replyCount"].toInt(), 2);
}

QTEST_APPLESS_MAIN(TestYtDlpTranslate)
#include "testytdlptranslate.moc"
