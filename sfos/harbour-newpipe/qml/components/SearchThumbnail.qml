import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import harbour.newpipe.extractor 1.0

Image {
    id: thumbnail

    property int infoType
    property bool pressed

    fillMode: Image.PreserveAspectCrop
    verticalAlignment: Image.AlignVCenter

    // Layer is used to show Channels
    layer.enabled: infoType === SearchItem.Channel
    layer.effect: OpacityMask {
        maskSource: Item {
            width: thumbnail.width
            height: thumbnail.height
            Rectangle {
                width: height
                height: parent.height
                anchors.centerIn: parent
                radius: width / 2.0
            }
        }
    }

    // Rectangle and Image used to show Playlists
    Rectangle {
        visible: (infoType === SearchItem.Playlist) && (thumbnail.status === Image.Ready)
        anchors.centerIn: parent
        height: parent.width
        width: parent.height
        color: "black"
        rotation: 90
        opacity: 0.8
        gradient: Gradient {
            GradientStop { position: 0.5; color: "black"; }
            GradientStop { position: 0.8; color: "transparent"; }
        }
    }
    Icon {
        visible: (infoType === SearchItem.Playlist) && (thumbnail.status === Image.Ready)
        x: (3.0 * parent.width / 4.0) - (width / 2.0)
        y: (parent.height - height) / 2.0
        anchors.centerIn: background
        width: Theme.iconSizeMedium
        height: Theme.iconSizeMedium
        fillMode: Image.PreserveAspectFit
        source: "image://theme/icon-m-media-playlists"
        highlighted: thumbnail.pressed
    }
}
