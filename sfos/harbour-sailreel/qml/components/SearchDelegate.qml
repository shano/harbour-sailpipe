import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

BackgroundItem {
    id: delegate

    property int infoType
    property url thumbnail
    property string name
    property string url
    property string infoRow

    focus: false
    height: thumbnail.height + (2 * Theme.paddingMedium)
    readonly property real iconScale: 1.5

    ListView.onAdd: AddAnimation {
        id: animadd
        target: delegate
    }
    ListView.onRemove: RemoveAnimation {
        id: animremove
        target: delegate
    }

    Row {
        x: Theme.horizontalPageMargin
        y: Theme.paddingMedium
        width: parent.width - (2 * Theme.horizontalPageMargin)
        height: thumbnail.height
        spacing: Theme.paddingLarge

        SearchThumbnail {
            id: thumbnail
            infoType: delegate.infoType
            source: delegate.thumbnail
            width: Theme.iconSizeLarge * iconScale
            height: Theme.iconSizeMedium * iconScale
            pressed: delegate.pressed
        }

        Column {
            width: parent.width - thumbnail.width - Theme.paddingLarge
            anchors.verticalCenter: parent.verticalCenter

            Label {
                color: delegate.pressed ? Theme.highlightColor : Theme.primaryColor
                textFormat: Text.PlainText
                text: name
                width: parent.width
                truncationMode: TruncationMode.Fade
                focus: false
            }

            Label {
                color: delegate.pressed ? Theme.secondaryHighlightColor : Theme.secondaryColor
                textFormat: Text.PlainText
                font.pixelSize: Theme.fontSizeExtraSmall
                text: infoRow || ""
                width: parent.width
                truncationMode: TruncationMode.Fade
                focus: false
                visible: !!infoRow
            }
        }
    }

    onClicked: {
        switch (infoType) {
            case SearchItem.Channel:
                pageStack.push(Qt.resolvedUrl("../pages/ChannelPage.qml"), {
                    name: name,
                    thumbnail: delegate.thumbnail,
                    url: url,
                    infoRow: infoRow
                });
                break;
            case SearchItem.Playlist:
                pageStack.push(Qt.resolvedUrl("../pages/PlaylistPage.qml"), {
                    name: name,
                    thumbnail: delegate.thumbnail,
                    url: url
                });
                break;
            case SearchItem.Stream:
                pageStack.push(Qt.resolvedUrl("../pages/VideoPage.qml"), {
                    name: name,
                    thumbnail: delegate.thumbnail,
                    url: url
                });
                break;
            default:
                console.log("Unknown search item type: " + infoType);
                break;
        }
    }
}
