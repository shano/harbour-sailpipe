#include <sailfishapp.h>
#include <QQuickImageProvider>
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <silicatheme.h>
#include <math.h>

#include "imageprovider.h"

ImageProvider::ImageProvider()
  : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
  Silica::Theme *silicaTheme = Silica::Theme::instance();
  double pixelRatio = silicaTheme->pixelRatio();

  double quantised = std::min(std::max(std::ceil(pixelRatio * 4.0) / 4.0, 1.0), 2.0);
  QString dir = QString::number(quantised, 'f', 2);
  if (dir.length() == 4 && dir.at(3) == '0') {
    dir.truncate(3);
  }

  m_imageDir = SailfishApp::pathTo("qml/images/z" + dir).toString(QUrl::RemoveScheme) + "/";
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    QPixmap image;
    QStringList parts = id.split('?');
    QPixmap sourcePixmap(m_imageDir + parts.at(0) + ".png");

    if (size) {
        *size = sourcePixmap.size();
    }
    if (parts.length() > 1) {
        if (QColor::isValidColor(parts.at(1))) {
            QPainter painter(&sourcePixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(sourcePixmap.rect(), parts.at(1));
            painter.end();
        }
    }
    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        image = sourcePixmap.scaled(requestedSize.width(), requestedSize.height(), Qt::IgnoreAspectRatio);
    }
    else {
        image = sourcePixmap;
    }

    return image;
}
