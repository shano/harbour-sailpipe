import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

Row {
    id: root

    readonly property int buttonsize: Theme.iconSizeSmallPlus
    property bool downloadable: false

    height: buttonsize
    spacing: 2 * Theme.paddingLarge
    layoutDirection: Qt.RightToLeft

    signal fullscreenPressed
    signal sharePressed
    signal downloadPressed
    signal downloadCancelPressed

    IconButton {
        width: parent.buttonsize
        height: parent.buttonsize
        icon.sourceSize.width: width
        icon.sourceSize.height: height
        icon.source: Qt.resolvedUrl("image://newpipe/icon-splus-scale?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

        onClicked: fullscreenPressed()
    }

    IconButton {
        width: parent.buttonsize
        height: parent.buttonsize
        icon.sourceSize.width: width
        icon.sourceSize.height: height
        icon.source: Qt.resolvedUrl("image://newpipe/icon-splus-share?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

        onClicked: sharePressed()
    }

    MouseArea {
        width: parent.buttonsize
        height: parent.buttonsize

        enabled: downloadable
        opacity: downloadable || (DownloadManager.downloadStatus === DownloadManager.Running) ? 1.0 : Theme.opacityLow

        property bool down: pressed && containsMouse
        property bool showPress: down || pressTimer.running

        Image {
            id: downloadButton
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            source: Qt.resolvedUrl("image://newpipe/icon-splus-cloud-download?") + (parent.showPress ? Theme.highlightColor : Theme.primaryColor)
            opacity: 1.0
        }
        Image {
            id: downloadDone
            property string image: "image://newpipe/icon-splus-download-success?"
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            source: Qt.resolvedUrl(image) + (parent.showPress ? Theme.highlightColor : Theme.primaryColor)
            opacity: 0.0

            Connections {
                target: DownloadManager
                onDownloadStatusChanged: {
                    switch (DownloadManager.downloadStatus) {
                    case DownloadManager.Error:
                        downloadDone.image = "image://newpipe/icon-splus-download-error?";
                        break;
                    case DownloadManager.Cancelled:
                        downloadDone.image = "image://newpipe/icon-splus-download-cancel?";
                        break;
                    case DownloadManager.Done:
                        downloadDone.image = "image://newpipe/icon-splus-download-success?";
                        break;
                    default:
                        // Do nothing;
                        break;
                    }
                }
            }
        }
        ProgressCircleBase {
            id: downloadProgress
            anchors.fill: parent
            progressColor: palette.primaryColor
            backgroundColor: Qt.rgba(palette.secondaryColor.r, palette.secondaryColor.g, palette.secondaryColor.b, Theme.opacityLow)
            value: DownloadManager.progress
            visible: false
        }

        onPressedChanged: {
            if (pressed) {
                pressTimer.start()
            }
        }
        onCanceled: pressTimer.stop()

        Timer {
            id: pressTimer
            interval: Theme.minimumPressHighlightTime
        }

        onClicked: {
            if (DownloadManager.downloadStatus === DownloadManager.Running) {
                downloadCancelPressed()
            }
            else {
                downloadPressed()
            }
        }
    }

    states: [
        State {
            name: "running"
            when: DownloadManager.downloadStatus === DownloadManager.Running
            PropertyChanges {
                target: downloadButton
                opacity: 0.0
            }
            PropertyChanges {
                target: downloadDone
                opacity: 0.0
            }
            PropertyChanges {
                target: downloadProgress
                visible: true
            }
        },
        State {
            name: "done"
            when: (DownloadManager.downloadStatus === DownloadManager.Done) || (DownloadManager.downloadStatus === DownloadManager.Error) || (DownloadManager.downloadStatus === DownloadManager.Cancelled)
            PropertyChanges {
                target: downloadButton
                opacity: 0.0
            }
            PropertyChanges {
                target: downloadDone
                opacity: 1.0
            }
            PropertyChanges {
                target: downloadProgress
                visible: false
            }
        }
    ]

    transitions: [
        Transition {
            from: "done"; to: ""
            NumberAnimation { target: downloadButton; properties: "opacity"; duration: 500; easing.type: Easing.InOutQuad}
            NumberAnimation { target: downloadDone; properties: "opacity"; duration: 500; easing.type: Easing.InOutQuad}
        }
    ]
}
