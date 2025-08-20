#include "videomanagement.h"

VideoManagement::VideoManagement(QObject *parent) : QObject(parent)
{

}

void VideoManagement::setMediaPlayerSource(QObject* mediaPlayer) const
{
  QVariant source = mediaPlayer->property("source");
}
