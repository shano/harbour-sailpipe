import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import harbour.newpipe.extractor 1.0

Item {
    id: root
    property alias source: media.source
    property alias thumbnail: image.source
    property bool controlsvisible: false
    property int skipTimeShort: 10
    property int skipTimeLong: 60

    readonly property int controlgap: 2 * Theme.paddingLarge
    readonly property bool playing: (media.playbackState == MediaPlayer.PlayingState)
    property bool controllable: isControllable()
    property bool forceVisible: controllable

    width: parent.width
    height: width * (9 / 16)

    function isControllable() {
        var result = true;
        switch (media.status) {
        case MediaPlayer.NoMedia:
        case MediaPlayer.Loading:
        case MediaPlayer.InvalidMedia:
        case MediaPlayer.UnknownStatus:
            result = false;
        }
        return result;
    }

    onForceVisibleChanged: {
        if ((forceVisible == true) && (controllable == true)) {
            openControls();
        }
    }

    function toggleControls() {
        if ((!controlsvisible) || (fadeout.running)) {
            fadeout.stop()
            controls.opacity = 1
            controlsvisible = true
            controlsTimer.restart()
        }
        else {
            controlsTimer.stop()
            fadeout.stop()
            controlsvisible = false
        }
    }

    function openControls () {
        fadeout.stop()
        controls.opacity = 1
        controlsvisible = true
        if (forceVisible) {
            controlsTimer.stop()
        }
        else {
            controlsTimer.restart()
        }
    }

    function closeControls () {
        controlsTimer.stop()
        fadeout.stop()
        controlsvisible = false
    }

    MediaPlayer {
        id: media
        autoPlay: false
        autoLoad: true
        onPositionChanged: {
            mediaslider.value = position
        }
        onStatusChanged: {
            if (status == MediaPlayer.EndOfMedia) {
                seek(0);
                forceVisible = true;
                openControls();
            }
        }
    }

    VideoOutput {
        id: video
        anchors.fill: parent
        width: parent.width
        height: width * (9 / 16)
        fillMode: Image.PreserveAspectFit
        source: media

        Image {
            id: image
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            visible: !controllable || forceVisible || !media.hasVideo
        }
    }

    Timer {
        id: controlsTimer
        interval: 10000
        running: false
        repeat: false
        triggeredOnStart: false
        onTriggered: {
            fadeout.start()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            toggleControls()
        }
    }

    Item {
        id: controls
        anchors.fill: parent
        visible: (controllable && controlsvisible) || forceVisible
        opacity: 1

        NumberAnimation on opacity {
            id: fadeout
            from: 1
            to: 0
            duration: 1000
            onRunningChanged: {
                if (!running) {
                    controlsvisible = false
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                toggleControls()
            }
        }

        IconButton {
            id: playbutton
            width: Theme.iconSizeLarge
            height: Theme.iconSizeLarge
            icon.sourceSize.width: width
            icon.sourceSize.height: height
            icon.fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
            anchors.bottomMargin: controlgap
            anchors.horizontalCenter: parent.horizontalCenter
            icon.source: (playing ? Qt.resolvedUrl("image://theme/icon-l-pause?") : Qt.resolvedUrl("image://theme/icon-l-play?"))
                    + (pressed ? Theme.highlightColor : Theme.primaryColor)

            onClicked: {
                if (forceVisible) {
                    forceVisible = false;
                    closeControls();
                }
                else {
                    openControls()
                }
                if (playing) {
                    media.pause()
                }
                else {
                    media.play()
                }
            }
        }

        IconButtonDual {
            id: reversebutton
            width: Theme.iconSizeLarge
            height: Theme.iconSizeLarge
            anchors.verticalCenter: playbutton.verticalCenter
            anchors.right: playbutton.left
            anchors.rightMargin: controlgap
            icon.source: Qt.resolvedUrl("image://newpipe/icon-l-replay?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

            onShortClick: {
                openControls()
                media.seek(media.position - (1000 * skipTimeShort))
                mediaslider.value = mediaslider.value - (1000 * skipTimeShort)
            }
            onLongClick: {
                openControls()
                media.seek(media.position - (1000 * skipTimeLong))
                mediaslider.value = mediaslider.value - (1000 * skipTimeLong)
            }
        }

        IconButtonDual {
            id: forwardsbutton
            width: Theme.iconSizeLarge
            height: Theme.iconSizeLarge
            anchors.verticalCenter: playbutton.verticalCenter
            anchors.left: playbutton.right
            anchors.leftMargin: controlgap
            icon.source: Qt.resolvedUrl("image://newpipe/icon-l-skip?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

            onShortClick: {
                openControls()
                media.seek(media.position + (1000 * skipTimeShort))
                mediaslider.value = mediaslider.value + (1000 * skipTimeShort)
            }
            onLongClick: {
                openControls()
                media.seek(media.position + (1000 * skipTimeLong))
                mediaslider.value = mediaslider.value + (1000 * skipTimeLong)
            }
        }

        Slider {
            id: mediaslider
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            minimumValue: 0
            maximumValue: Math.max(1, media.duration)
            stepSize: 1
            value: 0
            enabled: media.seekable
            valueText: Utils.millisecondsToTime(sliderValue)
            onPressed: {
                openControls()
                controlsTimer.stop()
            }
            onReleased: {
                media.seek(sliderValue)
                openControls()
            }
        }

        IconButton {
            id: fullscreenbutton
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
            icon.sourceSize.width: width
            icon.sourceSize.height: height
            icon.fillMode: Image.PreserveAspectFit
            anchors.right: parent.right
            anchors.rightMargin: Theme.paddingLarge
            anchors.top: parent.top
            anchors.topMargin: Theme.paddingLarge
            icon.source: Qt.resolvedUrl("image://theme/icon-m-scale?") + (pressed ? Theme.highlightColor : Theme.primaryColor)

            onClicked: {
                console.log("Pressed: fullscreen");
                pageStack.push(Qt.resolvedUrl("../pages/FullscreenVideoPage.qml"), {media: media});
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: true
        visible: !controllable
        size: BusyIndicatorSize.Medium
    }
}
