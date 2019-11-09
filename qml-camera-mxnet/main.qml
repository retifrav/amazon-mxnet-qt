import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtMultimedia 5.12
import io.qt.Backend 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 900
    minimumWidth: 700
    height: 500
    minimumHeight: 400
    title: "Camera test"

    property bool cameraUpsideDown: false // if you need to rotate viewfinder to 180
    property int currentFPSvalue_camera: 0

    Backend {
        id: backend

        onRequestDone: {
            //console.log(result);
        }

        onRequestFailed: {
            //console.log("Error:", error);
        }

        onCounterIncreased: {
            currentFPSvalue_camera++;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#ECECEC"

        Text {
            id: cameraStatus
            anchors.centerIn: parent
            width: parent.width
            text: "loading camera..."
            font.pixelSize: 40
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Camera {
            id: camera
            // NVIDIA Jetson TX2: QT_GSTREAMER_CAMERABIN_VIDEOSRC="nvcamerasrc ! nvvidconv" ./camera-test
            deviceId: "/dev/video0"
            viewfinder.resolution: Qt.size(backend.frameWidth(), backend.frameHeight()) // picture quality
            metaData.orientation: cameraUpsideDown ? 180 : 0

            imageCapture {
                onCaptureFailed: {
                    //console.log("Some error taking a picture\n", message);
                }
                onImageCaptured: {
                    //console.log("photo has been captured")
                }
                onImageSaved: {
                    //console.log("photo has been saved:", camera.imageCapture.capturedImagePath)
                }
            }

            onError: {
                cameraStatus.text = "Error: " + errorString;
                console.log(errorCode, errorString);
            }

            Component.onCompleted: {
                console.log("Current camera:", camera.displayName, "|", camera.deviceId);

                //console.log("camera orientation:", camera.orientation);
                //console.log("camera state:", camera.cameraState);
                //console.log("camera status:", camera.cameraStatus);
                //console.log("ImageCapture supported resolutions:", imageCapture.supportedResolutions);

                var supRezes = camera.supportedViewfinderResolutions();
                console.log("Viewfinder supported resolutions:");
                for (var rez in supRezes)
                {
                    console.log("-", supRezes[rez].width, "x", supRezes[rez].height);
                }
            }
        }

        Binding {
            target: backend.videoWrapper
            property: "source"
            value: camera
        }

        VideoOutput {
            anchors.fill: parent
            orientation: cameraUpsideDown ? 180 : 0
            //fillMode: VideoOutput.PreserveAspectCrop
            source: backend.videoWrapper//camera

            // FPS counters
            Text {
                id: fpsCounter_camera
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 10
                anchors.leftMargin: 15
                text: "0"
                font.pointSize: 40
                color: "yellow"
            }
        }

        /*
        ListView {
            anchors.fill: parent
            anchors.margins: 10
            model: QtMultimedia.availableCameras
            delegate: Text {
                text: modelData.displayName + " | " + modelData.deviceId
                font.pixelSize: 16
            }
        }
        */
    }

    Timer {
        id: tm_sendFrame
        running: true
        repeat: true
        interval: 0.05 * 1000 // ms, the rate of grabbing frames (0.05 * 1000 = 20 FPS)
        onTriggered: {
            backend.enableSendingToMXNet(true);
        }
    }

    Timer {
        id: tm_fpsCounter
        running: true
        repeat: true
        interval: 1000
        onTriggered: {
            fpsCounter_camera.text = currentFPSvalue_camera;
            currentFPSvalue_camera = 0;
        }
    }
}
