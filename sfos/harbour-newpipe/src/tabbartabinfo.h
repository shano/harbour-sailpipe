#ifndef TABBARTABINFO_H
#define TABBARTABINFO_H

#include <QObject>

class ChannelModel;

class TabBarTabInfo : public QObject
{
  Q_OBJECT
public:
  explicit TabBarTabInfo(QObject *parent = nullptr);

  void setTitle(QString const& title);
  void setIcon(QString const& icon);
  void setNoitems(QString const& noitems);
  void setComponent(QObject* component);
  void setModel(ChannelModel* model);

  QString title() const;
  QString icon() const;
  QString noitems() const;
  QObject* component() const;
  ChannelModel* model() const;

signals:

private:
  QString m_title;
  QString m_icon;
  QString m_noitems;
  QObject* m_component;
  ChannelModel* m_model;
};

#endif // TABBARTABINFO_H
