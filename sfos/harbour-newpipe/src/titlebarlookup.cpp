#include "titlebarlookup.h"

TitleBarLookup::TitleBarLookup(QObject *parent)
  : QObject(parent)
{
}

TitleBarLookup::TitleBarLookup(QString const title, QString const icon, QObject* parent)
  : QObject(parent)
  , m_title(title)
  , m_icon(icon)
{
}

QString TitleBarLookup::title() const
{
  return m_title;
}

QString TitleBarLookup::icon() const
{
  return m_icon;
}
