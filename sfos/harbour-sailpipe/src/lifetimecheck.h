#ifndef LIFETIMECHECK_H
#define LIFETIMECHECK_H

#include <QObject>

class LifetimeCheck : public QObject
{
  Q_OBJECT
public:
  explicit LifetimeCheck(QObject const* subject, QObject* parent = nullptr);

  bool destroyed() const;

private slots:
  void onDestroyed();

private:
  bool m_destroyed;
};

#endif // LIFETIMECHECK_H
