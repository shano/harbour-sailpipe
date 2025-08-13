#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QAbstractListModel>
#include <QObject>

class Extractor;

class FilterModel : public QAbstractListModel
{
  Q_PROPERTY(QString defaultFilter READ defaultFilter NOTIFY defaultFilterChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)

  Q_OBJECT
public:
  enum FilterRoles {
    NameRole = Qt::UserRole + 1,
    FilterRole,
  };

  explicit FilterModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const;

  int rowCount(const QModelIndex & parent = QModelIndex()) const;

  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

  void replaceAll(QStringList const& filterResults);

  QString defaultFilter() const;
  int count() const;

private:
  static QString filterToName(QString const& filter);

public slots:
  void populate(Extractor* extractor);
  bool filterValid(QString const& filter) const;

signals:
  void defaultFilterChanged();
  void countChanged();

private:
  bool m_loading;
  QHash<int, QByteArray> m_roles;
  QStringList m_filterData;
  QString m_defaultFilter;
};

#endif // FILTERMODEL_H
