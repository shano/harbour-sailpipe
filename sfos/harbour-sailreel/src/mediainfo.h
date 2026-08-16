#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QObject>
#include <QDateTime>
#include <QMetaMethod>

class MediaInfo : public QObject
{
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(QString uploaderName READ uploaderName WRITE setUploaderName NOTIFY uploaderNameChanged)
  Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
  Q_PROPERTY(qint64 viewCount READ viewCount WRITE setViewCount NOTIFY viewCountChanged)
  Q_PROPERTY(qint64 likeCount READ likeCount WRITE setLikeCount NOTIFY likeCountChanged)
  Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
  Q_PROPERTY(QDateTime uploadDate READ uploadDate WRITE setUploadDate NOTIFY uploadDateChanged)
  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
  Q_PROPERTY(DescType descriptionType READ descriptionType WRITE setDescriptionType NOTIFY descriptionTypeChanged)
  Q_PROPERTY(qint64 length READ length WRITE setLength NOTIFY lengthChanged)
  Q_PROPERTY(QString licence READ licence WRITE setLicence NOTIFY licenceChanged)

  Q_OBJECT
public:
  enum DescType {
    HTML = 1,
    Markdown= 2,
    PlainText = 3,
  };
  Q_ENUM(DescType)

  explicit MediaInfo(QObject *parent = nullptr);
  explicit MediaInfo(QString const& url, QString const& name, QString const& uploaderName, QString const& category, qint64 viewCount, qint64 likeCount, QString const& content, QDateTime const& uploadDate, QString description, DescType descriptionType, quint64 length, QString licence, QObject *parent = nullptr);
  explicit MediaInfo(QJsonObject const& json, QObject *parent = nullptr);

public slots:
  QString name() const;
  QString uploaderName() const;
  QString category() const;
  qint64 viewCount() const;
  qint64 likeCount() const;
  QString content() const;
  QDateTime uploadDate() const;
  QString description() const;
  DescType descriptionType() const;
  quint64 length() const;
  QString licence() const;

  void setName(QString const& name);
  void setUploaderName(QString const& uploaderName);
  void setCategory(QString const& category);
  void setViewCount(qint64 viewCount);
  void setLikeCount(qint64 likeCount);
  void setContent(QString const& content);
  void setUploadDate(QDateTime const& uploadDate);
  void setDescription(QString const& description);
  void setDescriptionType(DescType descriptionType);
  void setLength(quint64 length);
  void setLicence(QString const& licence);

  void parseJson(QJsonObject const& json);

private:
  typedef void (MediaInfo::*MediaInfoSignal)();

  QList<MediaInfoSignal> parseJsonChanges(QJsonObject const& json);

signals:
  void nameChanged();
  void uploaderNameChanged();
  void categoryChanged();
  void viewCountChanged();
  void likeCountChanged();
  void contentChanged();
  void uploadDateChanged();
  void descriptionChanged();
  void descriptionTypeChanged();
  void lengthChanged();
  void licenceChanged();

private:
  QString m_name;
  QString m_uploaderName;
  QString m_category;
  qint64 m_viewCount;
  qint64 m_likeCount;
  QString m_content;
  QDateTime m_uploadDate;
  QString m_description;
  DescType m_descriptionType;
  quint64 m_length;
  QString m_licence;
};

#endif // MEDIAINFO_H
