#include "tabbarlookup.h"

TabBarLookup::TabBarLookup(QObject *parent)
  : QObject(parent)
{
}

TabBarLookup::TabBarLookup(QString const title, QString const icon, QObject* parent)
  : QObject(parent)
  , m_title(title)
  , m_icon(icon)
{
}

QString TabBarLookup::title() const
{
  return m_title;
}

QString TabBarLookup::icon() const
{
  return m_icon;
}
