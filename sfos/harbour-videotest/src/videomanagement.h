#ifndef VIDEOMANAGEMENT_H
#define VIDEOMANAGEMENT_H

#include <QObject>

class VideoManagement : public QObject
{
  Q_OBJECT
public:
  explicit VideoManagement(QObject *parent = nullptr);

public slots:
  void setMediaPlayerSource(QObject* mediaPlayer) const;

signals:

};

#endif // VIDEOMANAGEMENT_H
