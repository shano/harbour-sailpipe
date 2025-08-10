#include "tabbartabinfo.h"

TabBarTabInfo::TabBarTabInfo(QObject *parent)
  : QObject(parent)
  , m_title()
  , m_icon()
  , m_noitems()
  , m_component(nullptr)
  , m_model(nullptr)
{

}

void TabBarTabInfo::setTitle(QString const& title)
{
  m_title = title;
}

void TabBarTabInfo::setIcon(QString const& icon)
{
  m_icon = icon;
}

void TabBarTabInfo::setNoitems(const QString &noitems)
{
  m_noitems = noitems;
}

void TabBarTabInfo::setComponent(QObject* component)
{
  m_component = component;
}

void TabBarTabInfo::setModel(ChannelModel* model)
{
  m_model = model;
}

QString TabBarTabInfo::title() const
{
  return m_title;
}

QString TabBarTabInfo::icon() const
{
  return m_icon;
}

QString TabBarTabInfo::noitems() const
{
  return m_noitems;
}

QObject* TabBarTabInfo::component() const
{
  return m_component;
}

ChannelModel* TabBarTabInfo::model() const
{
  return m_model;
}
