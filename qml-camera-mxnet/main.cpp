#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QCameraInfo>
#include <QDir>
#include "backend.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //qputenv("QT_QPA_PLATFORM", "webgl:port=8998");

    QGuiApplication app(argc, argv);

    qDebug() << "cameras found:";
    QListIterator<QCameraInfo> cameras(QCameraInfo::availableCameras());
    while(cameras.hasNext())
    {
        qDebug() << "-" << cameras.next();
    }

    QQmlApplicationEngine engine;
    qmlRegisterType<Backend>("io.qt.Backend", 1, 0, "Backend");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) { return -1; }

    return app.exec();
}
