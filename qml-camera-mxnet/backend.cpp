#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent)
{
    videoWrapper = new VideoWrapper();
    connect(
            videoWrapper, &VideoWrapper::gotNewFrameImage,
            this, &Backend::processFrame
            );
    connect(
            videoWrapper, &VideoWrapper::gotNewFrame,
            this, [=](){ this->counterIncreased(); }
            );

    managerFrames = new QNetworkAccessManager(this);
    connect(
            managerFrames, &QNetworkAccessManager::finished,
            this, &Backend::processFrameFinished
            );
}

Backend::~Backend()
{
    // close connections or whatever
}

VideoWrapper *Backend::get_videoWrapper()
{
    return videoWrapper;
}

int Backend::frameWidth() const
{
    return _frameWidth;
}

int Backend::frameHeight() const
{
    return _frameHeight;
}

void Backend::processFrame(QImage img)
{
    // 384x288 is enough for MXNet
    QImage shotScaled = img.scaled(384, 288);
    // MXNet should get unmirrored frame
    shotScaled = shotScaled.mirrored(true, false);

    //qDebug() << shotScaled.save(QString("%1-some.jpg").arg(QDateTime::currentDateTime().toString("hh-mm-ss-zzz")));

    QBuffer *imgBuffer = new QBuffer();
    //QImageWriter iw(imgBuffer, "JPG");
    //bool shotSaved = iw.write(shot);
    bool shotSaved = shotScaled.save(imgBuffer, "JPG");
    if (!shotSaved) { qCritical() << "[error] Couldn't save the original shot]" << imgBuffer->errorString(); }
    else
    {
        const QByteArray reqID = QUuid::createUuid().toByteArray();
        _frames.insert(reqID, img);

        //qDebug() << "uploading" << imgBuffer->size() << "bytes";
        QNetworkRequest request = QNetworkRequest(QUrl(_mxnetEndpointPose));
        request.setRawHeader("Content-Type", "multipart/form-data");
        request.setRawHeader("reqID", reqID);

        imgBuffer->open(QIODevice::ReadOnly);
        QNetworkReply *reply = managerFrames->post(request, imgBuffer);
        connect(reply, &QNetworkReply::finished, imgBuffer, &QBuffer::deleteLater);
    }
}

void Backend::processFrameFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    //qDebug() << status << "|" << data;

    if (status != 200)
    {
        QString errorMessage = data;
        QNetworkReply::NetworkError err = reply->error();
        if (status == 0)
        {
            // dictionary: http://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
            errorMessage = QString("QNetworkReply::NetworkError code: %1").arg(QString::number(err));
        }

        emit requestFailed(QString("Code %1 | %2").arg(status).arg(errorMessage));
        return;
    }

    // original frame is no longer needed
    QByteArray reqID = reply->request().rawHeader("reqID");
    _frames.remove(reqID);

    emit requestDone(data);
}

void Backend::enableSendingToMXNet(bool sendingEnabled)
{
    videoWrapper->enableSending(sendingEnabled);
}
