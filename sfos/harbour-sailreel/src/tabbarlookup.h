#ifndef TABBARLOOKUP_H
#define TABBARLOOKUP_H

#include <QObject>

class TabBarLookup : public QObject
{
  Q_OBJECT
public:
  explicit TabBarLookup(QObject *parent = nullptr);
  explicit TabBarLookup(QString const& title, QString const& icon, QString const& noitems, QObject* parent);

  QString title() const;
  QString icon() const;
  QString noitems() const;
signals:

private:
  QString const m_title;
  QString const m_icon;
  QString const m_noitems;
};

#endif // TABBARLOOKUP_H
