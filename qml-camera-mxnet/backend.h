#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QNetworkProxy>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "videowrapper.h"

class Backend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject *videoWrapper READ get_videoWrapper CONSTANT)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();

signals:
    void counterIncreased();
    QString requestDone(QString result);
    QString requestFailed(QString error);

public slots:
    VideoWrapper *get_videoWrapper();
    void processFrame(QImage img);

    int frameWidth() const;
    int frameHeight() const;

    void enableSendingToMXNet(bool sendingEnabled);

private slots:
    void processFrameFinished(QNetworkReply *reply);

private:
    VideoWrapper *videoWrapper;
    QHash<QByteArray, QImage> _frames;
    // frame width for the camera's resolution
    const int _frameWidth = 640;
    // frame height for the camera's resolution
    const int _frameHeight = 480;
    // MXNet
    const QString _mxnetEndpointPose = "http://localhost:8080/predictions/pose";
    const QString _mxnetEndpointHand = "http://localhost:8080/predictions/hand";
    QNetworkAccessManager *managerFrames;
};

#endif // BACKEND_H
