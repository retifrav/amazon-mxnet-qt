#include "videowrapper.h"

QList<QVideoFrame::PixelFormat> VideoWrapper
    ::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_UYVY << QVideoFrame::Format_BGR32;
}

bool VideoWrapper::present(const QVideoFrame &frame)
{
    if (surf) { surf->present(frame); }

    if (_sendingEnabled)
    {
        _sendingEnabled = false;

        QVideoFrame frameCopy = frame;
        frameCopy.map(QAbstractVideoBuffer::ReadOnly);
        //qDebug() << "got frame:" << frameCopy.mappedBytes() << frameCopy.size();
        QImage shot = QImage(qt_imageFromVideoFrame(frameCopy));
        frameCopy.unmap();

        emit gotNewFrameImage(shot);
    }

    emit gotNewFrame();

    return true;
}

bool VideoWrapper::start(const QVideoSurfaceFormat &format)
{
    m_format = format;
    // we need mirror the user hands
    m_format.setMirrored(true);

    if(supportedPixelFormats().count(m_format.pixelFormat()) == 0)
        { return false; }

    if(!QAbstractVideoSurface::start(m_format))
        { return false; }

    if( surf && !surf->start(m_format) )
        { return false; }

    return true;
}

void VideoWrapper::stop()
{
    QAbstractVideoSurface::stop();
}

QAbstractVideoSurface *VideoWrapper::get_videoSurface() const
{
    return surf;
}

void VideoWrapper::set_videoSurface(QAbstractVideoSurface *m)
{
    if (surf == m) { return; }
    if (surf && surf->isActive()) { surf->stop(); }

    surf = m;
    if (surf && isActive()) { surf->start(m_format); }
}

QObject *VideoWrapper::get_source()
{
    return m_source;
}

void VideoWrapper::set_source(QObject *qsrc)
{
    m_source = qsrc;

    if (qsrc)
    {
        QCamera *camera = qvariant_cast<QCamera*>(qsrc->property("mediaObject"));
        if(camera)
        {
            camera->setCaptureMode(QCamera::CaptureViewfinder);
            camera->setViewfinder(this);
        }
        else
        {
            qsrc->setProperty("videoSurface", QVariant::fromValue<QAbstractVideoSurface*>(this));
        }
    }

    emit sourceChanged();
}

void VideoWrapper::enableSending(bool sendingEnabled)
{
    _sendingEnabled = sendingEnabled;
}
