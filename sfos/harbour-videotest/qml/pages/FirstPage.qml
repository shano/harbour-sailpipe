import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Video test")
            }

            MediaPlayer {
                id: media
                objectName: "media"
                autoPlay: true
                autoLoad: true
                source: "https://download.blender.org/peach/bigbuckbunny_movies/BigBuckBunny_320x180.mp4"
            }

            VideoOutput {
                id: video00
                objectName: "video00"
                width: parent.width
                height: width
                fillMode: Image.PreserveAspectFit
                source: media

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "red"
                }
            }

            VideoOutput {
                id: video01
                objectName: "video01"
                width: parent.width
                height: width
                fillMode: Image.PreserveAspectFit
                source: media

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "green"
                }
            }
        }
    }
}
