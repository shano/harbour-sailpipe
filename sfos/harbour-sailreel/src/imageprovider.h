#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <sailfishapp.h>

class ImageProvider : public QQuickImageProvider {
public:
    explicit ImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // IMAGEPROVIDER_H
