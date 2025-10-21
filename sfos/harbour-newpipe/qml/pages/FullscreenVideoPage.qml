import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import harbour.newpipe.extractor 1.0

FullscreenContentPage {
    id: root
    property var video

    onStatusChanged: {
        if (status === PageStatus.Activating) {
            video.finaliseFullscreen(root);
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            pageStack.pop();
        }
    }
}
