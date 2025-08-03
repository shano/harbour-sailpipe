#ifndef TABBARLOOKUP_H
#define TABBARLOOKUP_H

#include <QObject>

class TabBarLookup : public QObject
{
  Q_OBJECT
public:
  explicit TabBarLookup(QObject *parent = nullptr);
  explicit TabBarLookup(QString const title, QString const icon, QObject* parent);

  QString title() const;
  QString icon() const;
signals:

private:
  QString const m_title;
  QString const m_icon;
};

#endif // TABBARLOOKUP_H
