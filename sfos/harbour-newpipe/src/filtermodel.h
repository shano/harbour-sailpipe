#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QAbstractListModel>
#include <QObject>

class Extractor;

class FilterModel : public QAbstractListModel
{
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

private:
  static QString filterToName(QString const& filter);

public slots:
  void populate(Extractor* extractor);

private:
  bool m_loading;
  QHash<int, QByteArray> m_roles;
  QStringList m_filterData;
};

#endif // FILTERMODEL_H
