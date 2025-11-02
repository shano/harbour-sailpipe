import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

ListItem {
    id: root

    property bool open: false
    property int horizontalMargin: Theme.horizontalPageMargin
    property int bottomMargin: Theme.paddingMedium
    property int collapsedHeight: Theme.itemSizeExtraLarge
    property int expandedHeight: Theme.fontSizeExtraSmall + Theme.paddingSmall + mainCommentText.contentHeight + (2 * Theme.paddingSmall)
    readonly property bool expandable: expandedHeight > collapsedHeight

    property string url
    property string uploaderAvatar
    property string uploaderName
    property string commentText
    property int replyCount
    property PageRef page

    width: parent ? parent.width : Screen.width
    contentHeight: collapsedHeight
    state: "collapsed"

    menu: menuComponent

    onClicked: {
        if (expandable) {
            open = !open
        }
    }

    Item {
        id: delegate
        focus: false
        anchors.fill: parent

        Row {
            id: commentRow
            x: Theme.paddingLarge
            y: Theme.paddingSmall
            width: parent.width - (2 * Theme.paddingLarge)
            height: parent.height - (2 * Theme.paddingSmall)
            spacing: Theme.paddingLarge

            Image {
                id: avatar
                y: Theme.paddingSmall
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                fillMode: Image.PreserveAspectFit
                verticalAlignment: Image.AlignTop
                source: uploaderAvatar
            }

            Column {
                id: comment
                width: parent.width - avatar.width - Theme.paddingLarge
                spacing: Theme.paddingSmall

                Label {
                    color: Theme.primaryColor
                    textFormat: Text.PlainText
                    width: parent.width
                    height: Theme.fontSizeExtraSmall
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: uploaderName
                    elide: Text.ElideRight
                    focus: false
                }

                Label {
                    id: mainCommentText
                    color: Theme.primaryColor
                    textFormat: Text.StyledText
                    width: parent.width
                    height: commentRow.height - Theme.fontSizeExtraSmall - Theme.paddingSmall
                    font.pixelSize: Theme.fontSizeSmall
                    text: commentText
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    focus: false
                }
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
            bottomMargin: Theme.paddingSmall
        }
        source: "image://theme/icon-lock-more"
    }

    OpacityRampEffect {
        property real ratio: expandable ? (expandedHeight - contentHeight) / (expandedHeight - collapsedHeight)
                                        : 1.0
        slope: 2 * Math.min(1.0, ratio)
        offset: 0.5
        sourceItem: delegate
        enabled: expandable && (state != "expanded")
        direction: OpacityRamp.TopToBottom
    }

    Component {
        id: menuComponent
        ContextMenu {
            MenuItem {
                //% "%n replies"
                text: qsTrId("sailpipe_comment_item-replies", replyCount)
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/RepliesPage.qml"), {
                        url: root.url,
                        uploaderAvatar: root.uploaderAvatar,
                        uploaderName: root.uploaderName,
                        commentText: root.commentText,
                        page: root.page,
                    });
                }
                enabled: (root.page.id) && (root.replyCount > 0)
            }
        }
    }

    states: [
        State {
            when: root.closed
            name: "collapsed"
            PropertyChanges {
                target: root
                contentHeight: collapsedHeight
            }
        },
        State {
            when: root.open
            name: "expanded"
            PropertyChanges {
                target: root
                contentHeight: expandedHeight
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            id: animation
            properties: "contentHeight"
            easing.type: Easing.InOutQuad
            duration: expandedHeight > Screen.height ? 400 : 200
        }
    }
}
