import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

CoverBackground {
    id: cover

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
    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: Utils.getImageUrl("icon-cover-replay")

            onTriggered: {
                console.log("Seek back 10000");
            }
        }

        CoverAction {
            iconSource: mediaavilable ? Qt.resolvedUrl("image://theme/icon-cover-pause") : Qt.resolvedUrl("image://theme/icon-cover-play")

            onTriggered: {
                console.log("Play/pause");
            }
        }
    }
}
