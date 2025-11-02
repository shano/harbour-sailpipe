#include <QDebug>

#include "lifetimecheck.h"

LifetimeCheck::LifetimeCheck(QObject const* subject, QObject* parent)
  : QObject(parent)
  , m_destroyed(false)
{
  // Ensure the subject isn't deleted before we use it
  // Connection will be severed if either this or the subject are destroyed
  QObject::connect(subject, &QObject::destroyed, this, &LifetimeCheck::onDestroyed);
}

bool LifetimeCheck::destroyed() const
{
  return m_destroyed;
}

void LifetimeCheck::onDestroyed()
{
  m_destroyed = true;
}
