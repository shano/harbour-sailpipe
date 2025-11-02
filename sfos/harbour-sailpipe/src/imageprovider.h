#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <sailfishapp.h>

class ImageProvider : public QQuickImageProvider {
public:
    explicit ImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
private:
    QString m_imageDir;
};

#endif // IMAGEPROVIDER_H
