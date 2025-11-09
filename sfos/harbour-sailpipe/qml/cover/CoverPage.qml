import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

CoverBackground {
    id: cover

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter
        source: MediaJunction.thumbnail
        opacity: 0.25
        visible: MediaJunction.controllable
    }

    Image {
        id: background
        visible: true
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: sourceSize.height * width / sourceSize.width
        source: Qt.resolvedUrl("image://sailpipe/cover-background")
        opacity: 0.1
    }

    Column {
        id: info
        x: Theme.paddingLarge
        y: Theme.paddingLarge
        width: cover.width - 2 * x
        spacing: Theme.paddingLarge

        Label {
            color: Theme.highlightColor
            width: parent.width
            //% "SailPipe"
            text: qsTrId("sailpipe_cover-title")
            fontSizeMode: Text.VerticalFit
            font.pixelSize: Theme.fontSizeLarge
            wrapMode: Text.Wrap
            elide: Text.ElideNone
            maximumLineCount: 3
        }

        Label {
            color: Theme.highlightColor
            width: parent.width
            text: MediaJunction.creator
            fontSizeMode: Text.VerticalFit
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.NoWrap
            truncationMode: TruncationMode.Fade
            elide: Text.ElideNone
            maximumLineCount: 1
            visible: MediaJunction.controllable
        }

        Item {
            width: parent.width
            height: title.height
            visible: MediaJunction.controllable

            Label {
                id: title
                color: Theme.primaryColor
                width: parent.width
                text: MediaJunction.title
                fontSizeMode: Text.VerticalFit
                font.pixelSize: Theme.fontSizeMedium
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                onLineLaidOut: {
                    if (line.number == maximumLineCount - 1) {
                        line.width = parent.width * 4
                    }
                }
            }

            OpacityRampEffect {
                offset: 0.5
                sourceItem: title
                enabled: title.implicitWidth > title.width
            }
        }
    }

    Progress {
        x: Theme.paddingLarge
        anchors.top: info.bottom
        anchors.topMargin: Theme.paddingLarge
        width: parent.width - (2.0 * Theme.paddingLarge)
        height: Theme.itemSizeSmall / 2.0
        progress: MediaJunction.position / MediaJunction.duration
        running: MediaJunction.playing
        visible: MediaJunction.controllable
    }

    CoverActionList {
        id: coverAction
        enabled: MediaJunction.controllable

        CoverAction {
            iconSource: Utils.getImageUrl("icon-cover-replay")

            onTriggered: {
                console.log("Seek back");
                MediaJunction.skipBackwardsRequested();
            }
        }

        CoverAction {
            iconSource: MediaJunction.playing ? Qt.resolvedUrl("image://theme/icon-cover-pause") : Qt.resolvedUrl("image://theme/icon-cover-play")

            onTriggered: {
                console.log("Play/pause");
                MediaJunction.playPauseRequested();
            }
        }
    }
}
