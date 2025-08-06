#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "searchmodel.h"
#include <QObject>

class PlaylistModel : public SearchModel
{
  Q_OBJECT
  Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
public:
  explicit PlaylistModel(QObject *parent = nullptr);

  QString url() const;
  void setUrl(QString const& url);

public slots:
  virtual void search(Extractor* extractor) override;
  virtual void searchMore(Extractor* extractor) override;

signals:
  void urlChanged();

private:
  QString m_url;
};

#endif // PLAYLISTMODEL_H
