#ifndef SUBSCRIPTIONMANAGER_H
#define SUBSCRIPTIONMANAGER_H

#include <QAbstractListModel>
#include <QList>
#include <QString>

class QQmlEngine;
class QJSEngine;

class SubscriptionManager : public QAbstractListModel
{
  Q_OBJECT

public:
  enum SubscriptionRoles {
    UrlRole = Qt::UserRole + 1,
    NameRole,
    ThumbnailRole,
  };

  explicit SubscriptionManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static SubscriptionManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QString urlAt(int row) const;
  QString nameAt(int row) const;
  QString thumbnailAt(int row) const;

public slots:
  bool isSubscribed(QString const& url) const;
  void subscribe(QString const& url, QString const& name, QString const& thumbnail);
  void unsubscribe(QString const& url);
  void removeAt(int row);
  QStringList allUrls() const;

signals:
  void subscriptionsChanged();

private:
  struct Subscription {
    QString url;
    QString name;
    QString thumbnail;
  };

  void load();
  void save() const;
  static QString storagePath();

  static SubscriptionManager* m_instance;
  QList<Subscription> m_subscriptions;
};

#endif // SUBSCRIPTIONMANAGER_H
