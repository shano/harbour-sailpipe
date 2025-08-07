import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0
import "../components"

SilicaControl {
    id: root

    property bool open: false
    property int horizontalMargin: Theme.horizontalPageMargin
    property int bottomMargin: 2 * Theme.paddingLarge
    property int collapsedHeight: Theme.itemSizeExtraLarge
    property int expandedHeight: column.height + (2 * Theme.paddingSmall) + bottomMargin
    readonly property bool expandable: expandedHeight > collapsedHeight
    property PlaylistModel playlistModel

    signal clicked

    width: parent ? parent.width : Screen.width
    height: collapsedHeight
    state: "collapsed"

    highlighted: content.pressed && content.containsMouse

    onClicked: {
        if (expandable) {
            open = !open
        }
    }

    MouseArea {
        id: content
        anchors {
            fill: parent
            bottomMargin: root.bottomMargin
        }
        enabled: expandable
        onClicked: parent.clicked()

        Column {
            id: column
            x: Theme.paddingLarge
            y: Theme.paddingSmall
            width: parent.width - Theme.paddingLarge
            spacing: Theme.paddingSmall

            Row {
                spacing: Theme.paddingLarge

                StatItem {
                    source: "image://newpipe/icon-s-media-count?" + Theme.highlightColor
                    //% "%n items"
                    text: qsTrId("newpipe_playlist_detail-item-count", playlistModel.streamCount)
                }

                StatItem {
                    source: "image://theme/icon-s-time?" + Theme.highlightColor
                    text: Utils.lengthToTimeString(playlistModel.duration)
                }
            }

            KeyValue {
                //% "Uploader"
                key: qsTrId("newpipe_playlist_details-detail_uploader")
                value: playlistModel.uploaderName
            }

            KeyValue {
                //% "Description"
                key: qsTrId("newpipe_playlist_details-detail_description")
                value: playlistModel.description
            }
        }
    }

    Icon {
        id: icon
        opacity: expandable ? 1.0 : 0.0
        Behavior on opacity { FadeAnimator {}}
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: horizontalMargin
            bottomMargin: Theme.paddingLarge
        }
        source: "image://theme/icon-lock-more"
    }

    OpacityRampEffect {
        property real ratio: expandable ? (expandedHeight - height) / (expandedHeight - collapsedHeight)
                                        : 1.0
        slope: 2 * Math.min(1.0, ratio)
        offset: 0.5
        sourceItem: content
        enabled: expandable && ((root.state != "expanded") || transition.running)
        direction: OpacityRamp.TopToBottom
    }

    states: [
        State {
            when: root.closed
            name: "collapsed"
            PropertyChanges {
                target: root
                height: collapsedHeight
            }
        },
        State {
            when: root.open
            name: "expanded"
            PropertyChanges {
                target: root
                height: expandedHeight
            }
        }
    ]

    transitions: Transition {
        id: transition
        NumberAnimation {
            properties: "height"
            easing.type: Easing.InOutQuad
            duration: expandedHeight > Screen.height ? 400 : 200
        }
    }
}
