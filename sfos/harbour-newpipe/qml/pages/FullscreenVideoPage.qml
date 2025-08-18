import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import harbour.newpipe.extractor 1.0

FullscreenContentPage {
    id: root
    property alias media: video.source

    VideoOutput {
        id: video
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        orientation: 270
    }
}
