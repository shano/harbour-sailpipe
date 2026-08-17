import QtQuick 2.6
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root
    property string name
    property url thumbnail
    property string url
    property string infoRow
    property ChannelInfo channelInfo: ChannelInfo { }
    property LinkHandlerModel linkHandlerModel: LinkHandlerModel { }
    readonly property real iconScale: 1.5
    property alias tabListModel: tabs.model
    property bool subscribed: false

    Component.onCompleted: {
        tabListModel.addAboutTab(extractor, aboutTab);
        extractor.getChannelInfo(channelInfo, linkHandlerModel, url);
        subscribed = Subscriptions.isSubscribed(url);
    }

    Connections {
        target: extractor
        onExtracted: {
            tabListModel.generateModel(extractor, linkHandlerModel);
        }
    }

    Connections {
        target: Subscriptions
        onSubscriptionsChanged: {
            subscribed = Subscriptions.isSubscribed(root.url);
        }
    }

    Component {
        id: aboutTab

        ChannelAbout {
            description: root.channelInfo.description
            subscriberCount: root.channelInfo.subscriberCount
            verified: root.channelInfo.verified
            tags: root.channelInfo.tags
        }
    }

    TabView {
        id: tabs

        anchors.fill: parent
        currentIndex: 0
        tabBarPosition: Qt.AlignTop
        page: root

        header: Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge
            bottomPadding: Theme.paddingLarge

            PageHeader {
                id: header
                //% "YouTube Channel"
                title: qsTrId("sailpipe_channel-page_header_channel")
            }

            Row {
                x: Theme.paddingLarge
                width: parent.width - (2 * Theme.paddingLarge)
                spacing: Theme.paddingLarge

                SearchThumbnail {
                    id: thumbnail
                    infoType: SearchItem.Channel
                    source: root.thumbnail
                    width: Theme.iconSizeLarge * iconScale
                    height: Theme.iconSizeMedium * iconScale
                }

                Column {
                    width: parent.width - thumbnail.width - subscribeButton.width - (2 * Theme.paddingLarge)
                    anchors.verticalCenter: parent.verticalCenter

                    Label {
                        color: Theme.highlightColor
                        textFormat: Text.PlainText
                        text: root.name
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        color: Theme.secondaryHighlightColor
                        textFormat: Text.PlainText
                        font.pixelSize: Theme.fontSizeExtraSmall
                        text: root.infoRow || ""
                        width: parent.width
                        truncationMode: TruncationMode.Fade
                        visible: !!root.infoRow
                    }
                }

                IconButton {
                    id: subscribeButton
                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: root.subscribed
                        ? "image://theme/icon-m-favorite-selected"
                        : "image://theme/icon-m-favorite"

                    onClicked: {
                        if (root.subscribed) {
                            Subscriptions.unsubscribe(root.url);
                        } else {
                            Subscriptions.subscribe(root.url, root.name, root.thumbnail.toString());
                        }
                    }
                }
            }
        }

        model: ChannelTabListModel { }
    }
}

