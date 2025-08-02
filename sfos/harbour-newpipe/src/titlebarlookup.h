#ifndef TITLEBARLOOKUP_H
#define TITLEBARLOOKUP_H

#include <QObject>

class TitleBarLookup : public QObject
{
  Q_OBJECT
public:
  explicit TitleBarLookup(QObject *parent = nullptr);
  explicit TitleBarLookup(QString const title, QString const icon, QObject* parent);

  QString title() const;
  QString icon() const;
signals:

private:
  QString const m_title;
  QString const m_icon;
};

#endif // TITLEBARLOOKUP_H
