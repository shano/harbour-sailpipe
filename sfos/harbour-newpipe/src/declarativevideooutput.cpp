/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2012 Research In Motion
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "declarativevideooutput.h"

#include "private/qdeclarativevideooutput_render_p.h"
#include "private/qdeclarativevideooutput_window_p.h"
#include <private/qvideooutputorientationhandler_p.h>
#include <QtMultimedia/qmediaobject.h>
#include <QtMultimedia/qmediaservice.h>
#include <private/qmediapluginloader_p.h>
#include <QtCore/qloggingcategory.h>
#include <private/qdeclarativevideooutput_backend_p.h>

static void initResource() {
    Q_INIT_RESOURCE(qtmultimediaquicktools);
}

Q_LOGGING_CATEGORY(qLcVideo, "qt.multimedia.video")

DeclarativeVideoOutput::DeclarativeVideoOutput(QQuickItem *parent) :
    QQuickItem(parent),
    m_sourceType(NoSource),
    m_fillMode(PreserveAspectFit),
    m_geometryDirty(true),
    m_orientation(0),
    m_autoOrientation(false),
    m_screenOrientationHandler(0)
{
    initResource();
    setFlag(ItemHasContents, true);
}

DeclarativeVideoOutput::~DeclarativeVideoOutput()
{
    m_backend.reset();
    m_source.clear();
    _q_updateMediaObject();
}

void DeclarativeVideoOutput::setSource(QObject *source)
{
    qCDebug(qLcVideo) << "source is" << source;

    if (source == m_source.data())
        return;

    if (m_source && m_sourceType == MediaObjectSource) {
        disconnect(m_source.data(), 0, this, SLOT(_q_updateMediaObject()));
        disconnect(m_source.data(), 0, this, SLOT(_q_updateCameraInfo()));
    }

    if (m_backend)
        m_backend->releaseSource();

    m_source = source;

    if (m_source) {
        const QMetaObject *metaObject = m_source.data()->metaObject();

        int mediaObjectPropertyIndex = metaObject->indexOfProperty("mediaObject");
        if (mediaObjectPropertyIndex != -1) {
            const QMetaProperty mediaObjectProperty = metaObject->property(mediaObjectPropertyIndex);

            if (mediaObjectProperty.hasNotifySignal()) {
                QMetaMethod method = mediaObjectProperty.notifySignal();
                QMetaObject::connect(m_source.data(), method.methodIndex(),
                                     this, this->metaObject()->indexOfSlot("_q_updateMediaObject()"),
                                     Qt::DirectConnection, 0);

            }

            int deviceIdPropertyIndex = metaObject->indexOfProperty("deviceId");
            if (deviceIdPropertyIndex != -1) { // Camera source
                const QMetaProperty deviceIdProperty = metaObject->property(deviceIdPropertyIndex);

                if (deviceIdProperty.hasNotifySignal()) {
                    QMetaMethod method = deviceIdProperty.notifySignal();
                    QMetaObject::connect(m_source.data(), method.methodIndex(),
                                         this, this->metaObject()->indexOfSlot("_q_updateCameraInfo()"),
                                         Qt::DirectConnection, 0);

                }
            }

            m_sourceType = MediaObjectSource;
        } else if (metaObject->indexOfProperty("videoSurface") != -1) {
            // Make sure our backend is a QDeclarativeVideoRendererBackend
            m_backend.reset();
            createBackend(0);
            Q_ASSERT(m_backend);
#ifndef QT_NO_DYNAMIC_CAST
            Q_ASSERT(dynamic_cast<QDeclarativeVideoRendererBackend *>(m_backend.data()));
#endif
            QAbstractVideoSurface * const surface = m_backend->videoSurface();
            Q_ASSERT(surface);
            m_source.data()->setProperty("videoSurface",
                                         QVariant::fromValue<QAbstractVideoSurface*>(surface));
            m_sourceType = VideoSurfaceSource;
        } else {
            m_sourceType = NoSource;
        }
    } else {
        m_sourceType = NoSource;
    }

    _q_updateMediaObject();
    Q_EMIT sourceChanged();
}

//Q_GLOBAL_STATIC_WITH_ARGS(QMediaPluginLoader, videoBackendFactoryLoader,
//        (QDeclarativeVideoBackendFactoryInterface_iid, QLatin1String("video/declarativevideobackend"), Qt::CaseInsensitive))

bool DeclarativeVideoOutput::createBackend(QMediaService *service)
{
    bool backendAvailable = false;
/*
    Q_FOREACH (QObject *instance, videoBackendFactoryLoader()->instances(QLatin1String("declarativevideobackend"))) {
        if (QDeclarativeVideoBackendFactoryInterface *plugin = qobject_cast<QDeclarativeVideoBackendFactoryInterface*>(instance)) {
            m_backend.reset(plugin->create(this));
            if (m_backend && m_backend->init(service)) {
                backendAvailable = true;
                break;
            }
        }
    }

    if (!backendAvailable) {
        m_backend.reset(new QDeclarativeVideoRendererBackend(this));
        if (m_backend->init(service))
            backendAvailable = true;
    }

    // QDeclarativeVideoWindowBackend only works when there is a service with a QVideoWindowControl.
    // Without service, the QDeclarativeVideoRendererBackend should always work.
    if (!backendAvailable) {
        Q_ASSERT(service);
        m_backend.reset(new QDeclarativeVideoWindowBackend(this));
        if (m_backend->init(service))
            backendAvailable = true;
    }
*/
    if (!backendAvailable) {
        qWarning() << Q_FUNC_INFO << "Media service has neither renderer nor window control available.";
        m_backend.reset();
    } else if (!m_geometryDirty) {
        m_backend->updateGeometry();
    }

    if (m_backend) {
        m_backend->clearFilters();
        for (int i = 0; i < m_filters.count(); ++i)
            m_backend->appendFilter(m_filters[i]);
    }

    return backendAvailable;
}

void DeclarativeVideoOutput::_q_updateMediaObject()
{
    QMediaObject *mediaObject = 0;

    if (m_source)
        mediaObject = qobject_cast<QMediaObject*>(m_source.data()->property("mediaObject").value<QObject*>());

    qCDebug(qLcVideo) << "media object is" << mediaObject;

    if (m_mediaObject.data() == mediaObject)
        return;

    if (m_sourceType != VideoSurfaceSource)
        m_backend.reset();

    m_mediaObject.clear();
    m_service.clear();

    if (mediaObject) {
        if (QMediaService *service = mediaObject->service()) {
            if (createBackend(service)) {
                m_service = service;
                m_mediaObject = mediaObject;
            }
        }
    }

    _q_updateCameraInfo();
}

void DeclarativeVideoOutput::_q_updateCameraInfo()
{
    if (m_mediaObject) {
        const QCamera *camera = qobject_cast<const QCamera *>(m_mediaObject);
        if (camera) {
            QCameraInfo info(*camera);

            if (m_cameraInfo != info) {
                m_cameraInfo = info;

                // The camera position and orientation need to be taken into account for
                // the viewport auto orientation
                if (m_autoOrientation)
                    _q_screenOrientationChanged(m_screenOrientationHandler->currentOrientation());
            }
        }
    } else {
        m_cameraInfo = QCameraInfo();
    }
}

DeclarativeVideoOutput::FillMode DeclarativeVideoOutput::fillMode() const
{
    return m_fillMode;
}

void DeclarativeVideoOutput::setFillMode(FillMode mode)
{
    if (mode == m_fillMode)
        return;

    m_fillMode = mode;
    m_geometryDirty = true;
    update();

    Q_EMIT fillModeChanged(mode);
}

void DeclarativeVideoOutput::_q_updateNativeSize()
{
    if (!m_backend)
        return;

    QSize size = m_backend->nativeSize();
    if (!qIsDefaultAspect(m_orientation)) {
        size.transpose();
    }

    if (m_nativeSize != size) {
        m_nativeSize = size;

        m_geometryDirty = true;

        setImplicitWidth(size.width());
        setImplicitHeight(size.height());

        Q_EMIT sourceRectChanged();
    }
}

/* Based on fill mode and our size, figure out the source/dest rects */
void DeclarativeVideoOutput::_q_updateGeometry()
{
    const QRectF rect(0, 0, width(), height());
    const QRectF absoluteRect(x(), y(), width(), height());

    if (!m_geometryDirty && m_lastRect == absoluteRect)
        return;

    QRectF oldContentRect(m_contentRect);

    m_geometryDirty = false;
    m_lastRect = absoluteRect;

    if (m_nativeSize.isEmpty()) {
        //this is necessary for item to receive the
        //first paint event and configure video surface.
        m_contentRect = rect;
    } else if (m_fillMode == Stretch) {
        m_contentRect = rect;
    } else if (m_fillMode == PreserveAspectFit || m_fillMode == PreserveAspectCrop) {
        QSizeF scaled = m_nativeSize;
        scaled.scale(rect.size(), m_fillMode == PreserveAspectFit ?
                         Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding);

        m_contentRect = QRectF(QPointF(), scaled);
        m_contentRect.moveCenter(rect.center());
    }

    if (m_backend)
        m_backend->updateGeometry();

    if (m_contentRect != oldContentRect)
        Q_EMIT contentRectChanged();
}

void DeclarativeVideoOutput::_q_screenOrientationChanged(int orientation)
{
    // If the source is a camera, take into account its sensor position and orientation
    if (!m_cameraInfo.isNull()) {
        switch (m_cameraInfo.position()) {
        case QCamera::FrontFace:
            // Front facing cameras are flipped horizontally, compensate the mirror
            orientation += (360 - m_cameraInfo.orientation());
            break;
        case QCamera::BackFace:
        default:
            orientation += m_cameraInfo.orientation();
            break;
        }
    }

    setOrientation(orientation % 360);
}

int DeclarativeVideoOutput::orientation() const
{
    return m_orientation;
}

void DeclarativeVideoOutput::setOrientation(int orientation)
{
    // Make sure it's a multiple of 90.
    if (orientation % 90)
        return;

    // If there's no actual change, return
    if (m_orientation == orientation)
        return;

    // If the new orientation is the same effect
    // as the old one, don't update the video node stuff
    if ((m_orientation % 360) == (orientation % 360)) {
        m_orientation = orientation;
        Q_EMIT orientationChanged();
        return;
    }

    m_geometryDirty = true;

    // Otherwise, a new orientation
    // See if we need to change aspect ratio orientation too
    bool oldAspect = qIsDefaultAspect(m_orientation);
    bool newAspect = qIsDefaultAspect(orientation);

    m_orientation = orientation;

    if (oldAspect != newAspect) {
        m_nativeSize.transpose();

        setImplicitWidth(m_nativeSize.width());
        setImplicitHeight(m_nativeSize.height());

        // Source rectangle does not change for orientation
    }

    update();
    Q_EMIT orientationChanged();
}

bool DeclarativeVideoOutput::autoOrientation() const
{
    return m_autoOrientation;
}

void DeclarativeVideoOutput::setAutoOrientation(bool autoOrientation)
{
    if (autoOrientation == m_autoOrientation)
        return;

    m_autoOrientation = autoOrientation;
    if (m_autoOrientation) {
        m_screenOrientationHandler = new QVideoOutputOrientationHandler(this);
        connect(m_screenOrientationHandler, SIGNAL(orientationChanged(int)),
                this, SLOT(_q_screenOrientationChanged(int)));

        _q_screenOrientationChanged(m_screenOrientationHandler->currentOrientation());
    } else {
        disconnect(m_screenOrientationHandler, SIGNAL(orientationChanged(int)),
                   this, SLOT(_q_screenOrientationChanged(int)));
        m_screenOrientationHandler->deleteLater();
        m_screenOrientationHandler = 0;
    }

    Q_EMIT autoOrientationChanged();
}

QRectF DeclarativeVideoOutput::contentRect() const
{
    return m_contentRect;
}

QRectF DeclarativeVideoOutput::sourceRect() const
{
    // We might have to transpose back
    QSizeF size = m_nativeSize;
    if (!qIsDefaultAspect(m_orientation)) {
        size.transpose();
    }

    // No backend? Just assume no viewport.
    if (!m_nativeSize.isValid() || !m_backend) {
        return QRectF(QPointF(), size);
    }

    // Take the viewport into account for the top left position.
    // m_nativeSize is already adjusted to the viewport, as it originats
    // from QVideoSurfaceFormat::sizeHint(), which includes pixel aspect
    // ratio and viewport.
    const QRectF viewport = m_backend->adjustedViewport();
    Q_ASSERT(viewport.size() == size);
    return QRectF(viewport.topLeft(), size);
}

QPointF DeclarativeVideoOutput::mapNormalizedPointToItem(const QPointF &point) const
{
    qreal dx = point.x();
    qreal dy = point.y();

    if (qIsDefaultAspect(m_orientation)) {
        dx *= m_contentRect.width();
        dy *= m_contentRect.height();
    } else {
        dx *= m_contentRect.height();
        dy *= m_contentRect.width();
    }

    switch (qNormalizedOrientation(m_orientation)) {
        case 0:
        default:
            return m_contentRect.topLeft() + QPointF(dx, dy);
        case 90:
            return m_contentRect.bottomLeft() + QPointF(dy, -dx);
        case 180:
            return m_contentRect.bottomRight() + QPointF(-dx, -dy);
        case 270:
            return m_contentRect.topRight() + QPointF(-dy, dx);
    }
}

QRectF DeclarativeVideoOutput::mapNormalizedRectToItem(const QRectF &rectangle) const
{
    return QRectF(mapNormalizedPointToItem(rectangle.topLeft()),
                  mapNormalizedPointToItem(rectangle.bottomRight())).normalized();
}

QPointF DeclarativeVideoOutput::mapPointToSource(const QPointF &point) const
{
    QPointF norm = mapPointToSourceNormalized(point);

    if (qIsDefaultAspect(m_orientation))
        return QPointF(norm.x() * m_nativeSize.width(), norm.y() * m_nativeSize.height());
    else
        return QPointF(norm.x() * m_nativeSize.height(), norm.y() * m_nativeSize.width());
}

QRectF DeclarativeVideoOutput::mapRectToSource(const QRectF &rectangle) const
{
    return QRectF(mapPointToSource(rectangle.topLeft()),
                  mapPointToSource(rectangle.bottomRight())).normalized();
}

QPointF DeclarativeVideoOutput::mapPointToSourceNormalized(const QPointF &point) const
{
    if (m_contentRect.isEmpty())
        return QPointF();

    // Normalize the item source point
    qreal nx = (point.x() - m_contentRect.left()) / m_contentRect.width();
    qreal ny = (point.y() - m_contentRect.top()) / m_contentRect.height();

    const qreal one(1.0f);

    // For now, the origin of the source rectangle is 0,0
    switch (qNormalizedOrientation(m_orientation)) {
        case 0:
        default:
            return QPointF(nx, ny);
        case 90:
            return QPointF(one - ny, nx);
        case 180:
            return QPointF(one - nx, one - ny);
        case 270:
            return QPointF(ny, one - nx);
    }
}

QRectF DeclarativeVideoOutput::mapRectToSourceNormalized(const QRectF &rectangle) const
{
    return QRectF(mapPointToSourceNormalized(rectangle.topLeft()),
                  mapPointToSourceNormalized(rectangle.bottomRight())).normalized();
}

DeclarativeVideoOutput::SourceType DeclarativeVideoOutput::sourceType() const
{
    return m_sourceType;
}

QPointF DeclarativeVideoOutput::mapPointToItem(const QPointF &point) const
{
    if (m_nativeSize.isEmpty())
        return QPointF();

    // Just normalize and use that function
    // m_nativeSize is transposed in some orientations
    if (qIsDefaultAspect(m_orientation))
        return mapNormalizedPointToItem(QPointF(point.x() / m_nativeSize.width(), point.y() / m_nativeSize.height()));
    else
        return mapNormalizedPointToItem(QPointF(point.x() / m_nativeSize.height(), point.y() / m_nativeSize.width()));
}

QRectF DeclarativeVideoOutput::mapRectToItem(const QRectF &rectangle) const
{
    return QRectF(mapPointToItem(rectangle.topLeft()),
                  mapPointToItem(rectangle.bottomRight())).normalized();
}

QSGNode *DeclarativeVideoOutput::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    _q_updateGeometry();

    if (!m_backend)
        return 0;

    return m_backend->updatePaintNode(oldNode, data);
}

void DeclarativeVideoOutput::itemChange(QQuickItem::ItemChange change,
                                         const QQuickItem::ItemChangeData &changeData)
{
    if (m_backend)
        m_backend->itemChange(change, changeData);
}

void DeclarativeVideoOutput::releaseResources()
{
    if (m_backend)
        m_backend->releaseResources();
}

void DeclarativeVideoOutput::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(newGeometry);
    Q_UNUSED(oldGeometry);

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    // Explicitly listen to geometry changes here. This is needed since changing the position does
    // not trigger a call to updatePaintNode().
    // We need to react to position changes though, as the window backened's display rect gets
    // changed in that situation.
    _q_updateGeometry();
}

QQmlListProperty<QAbstractVideoFilter> DeclarativeVideoOutput::filters()
{
    return QQmlListProperty<QAbstractVideoFilter>(this, 0, filter_append, filter_count, filter_at, filter_clear);
}

void DeclarativeVideoOutput::filter_append(QQmlListProperty<QAbstractVideoFilter> *property, QAbstractVideoFilter *value)
{
    DeclarativeVideoOutput *self = static_cast<DeclarativeVideoOutput *>(property->object);
    self->m_filters.append(value);
    if (self->m_backend)
        self->m_backend->appendFilter(value);
}

int DeclarativeVideoOutput::filter_count(QQmlListProperty<QAbstractVideoFilter> *property)
{
    DeclarativeVideoOutput *self = static_cast<DeclarativeVideoOutput *>(property->object);
    return self->m_filters.count();
}

QAbstractVideoFilter *DeclarativeVideoOutput::filter_at(QQmlListProperty<QAbstractVideoFilter> *property, int index)
{
    DeclarativeVideoOutput *self = static_cast<DeclarativeVideoOutput *>(property->object);
    return self->m_filters.at(index);
}

void DeclarativeVideoOutput::filter_clear(QQmlListProperty<QAbstractVideoFilter> *property)
{
    DeclarativeVideoOutput *self = static_cast<DeclarativeVideoOutput *>(property->object);
    self->m_filters.clear();
    if (self->m_backend)
        self->m_backend->clearFilters();
}

void DeclarativeVideoOutput::_q_invalidateSceneGraph()
{
    if (m_backend)
        m_backend->invalidateSceneGraph();
}
