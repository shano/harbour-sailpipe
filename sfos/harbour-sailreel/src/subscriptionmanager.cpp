#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include "subscriptionmanager.h"

SubscriptionManager* SubscriptionManager::m_instance = nullptr;

SubscriptionManager::SubscriptionManager(QObject *parent)
  : QAbstractListModel(parent)
{
  load();
}

void SubscriptionManager::instantiate(QObject* parent)
{
  if (m_instance == nullptr) {
    m_instance = new SubscriptionManager(parent);
  }
}

SubscriptionManager& SubscriptionManager::getInstance()
{
  return *m_instance;
}

QObject* SubscriptionManager::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

QHash<int, QByteArray> SubscriptionManager::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[UrlRole] = "url";
  roles[NameRole] = "name";
  roles[ThumbnailRole] = "thumbnail";
  return roles;
}

int SubscriptionManager::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return m_subscriptions.count();
}

QVariant SubscriptionManager::data(const QModelIndex& index, int role) const
{
  QVariant result;

  if ((index.row() >= 0) && (index.row() < m_subscriptions.count())) {
    Subscription const& subscription = m_subscriptions[index.row()];
    switch (role) {
      case UrlRole:
        result = subscription.url;
        break;
      case NameRole:
        result = subscription.name;
        break;
      case ThumbnailRole:
        result = subscription.thumbnail;
        break;
    }
  }

  return result;
}

QString SubscriptionManager::urlAt(int row) const
{
  return ((row >= 0) && (row < m_subscriptions.count())) ? m_subscriptions[row].url : QString();
}

QString SubscriptionManager::nameAt(int row) const
{
  return ((row >= 0) && (row < m_subscriptions.count())) ? m_subscriptions[row].name : QString();
}

QString SubscriptionManager::thumbnailAt(int row) const
{
  return ((row >= 0) && (row < m_subscriptions.count())) ? m_subscriptions[row].thumbnail : QString();
}

QStringList SubscriptionManager::allUrls() const
{
  QStringList urls;
  for (Subscription const& subscription : m_subscriptions) {
    urls.append(subscription.url);
  }
  return urls;
}

bool SubscriptionManager::isSubscribed(QString const& url) const
{
  for (Subscription const& subscription : m_subscriptions) {
    if (subscription.url == url) {
      return true;
    }
  }
  return false;
}

void SubscriptionManager::subscribe(QString const& url, QString const& name, QString const& thumbnail)
{
  if (isSubscribed(url)) {
    return;
  }

  beginInsertRows(QModelIndex(), m_subscriptions.count(), m_subscriptions.count());
  Subscription subscription;
  subscription.url = url;
  subscription.name = name;
  subscription.thumbnail = thumbnail;
  m_subscriptions.append(subscription);
  endInsertRows();

  save();
  emit subscriptionsChanged();
}

void SubscriptionManager::unsubscribe(QString const& url)
{
  for (int row = 0; row < m_subscriptions.count(); ++row) {
    if (m_subscriptions[row].url == url) {
      removeAt(row);
      return;
    }
  }
}

void SubscriptionManager::removeAt(int row)
{
  if ((row < 0) || (row >= m_subscriptions.count())) {
    return;
  }

  beginRemoveRows(QModelIndex(), row, row);
  m_subscriptions.removeAt(row);
  endRemoveRows();

  save();
  emit subscriptionsChanged();
}

QString SubscriptionManager::storagePath()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return dataDir + QStringLiteral("/subscriptions.json");
}

void SubscriptionManager::load()
{
  QFile file(storagePath());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QJsonArray array = doc.array();

  beginResetModel();
  m_subscriptions.clear();
  for (QJsonValue const& value : array) {
    QJsonObject object = value.toObject();
    Subscription subscription;
    subscription.url = object[QStringLiteral("url")].toString();
    subscription.name = object[QStringLiteral("name")].toString();
    subscription.thumbnail = object[QStringLiteral("thumbnail")].toString();
    if (!subscription.url.isEmpty()) {
      m_subscriptions.append(subscription);
    }
  }
  endResetModel();
}

void SubscriptionManager::save() const
{
  QString path = storagePath();
  QDir().mkpath(QFileInfo(path).absolutePath());

  QJsonArray array;
  for (Subscription const& subscription : m_subscriptions) {
    QJsonObject object;
    object[QStringLiteral("url")] = subscription.url;
    object[QStringLiteral("name")] = subscription.name;
    object[QStringLiteral("thumbnail")] = subscription.thumbnail;
    array.append(object);
  }

  QFile file(path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
  }
}
