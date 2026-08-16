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

    Component.onCompleted: {
        tabListModel.addAboutTab(extractor, aboutTab);
        extractor.getChannelInfo(channelInfo, linkHandlerModel, url);
    }

    Connections {
        target: extractor
        onExtracted: {
            tabListModel.generateModel(extractor, linkHandlerModel);
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
                title: {
                    var title = "";
                    switch (extractor.service) {
                    case Extractor.YouTubeService:
                    case Extractor.MediaCCCService:
                    case Extractor.PeertubeService:
                        //% "%0 Channel"
                        title = qsTrId("sailpipe_channel-page_header_channel").arg(extractor.serviceName);
                        break;
                    case Extractor.SoundcloudService:
                        //% "%0 User"
                        title = qsTrId("sailpipe_channel-page_header_user").arg(extractor.serviceName);
                        break;
                    case Extractor.BandcampService:
                        //% "%0 Artist"
                        title = qsTrId("sailpipe_channel-page_header_artist").arg(extractor.serviceName);
                        break;
                    default:
                        //% "Channel"
                        title = qsTrId("sailpipe_channel-page_header_channel_default");
                        break;
                    }
                    return title;
                }
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
                    width: parent.width - thumbnail.width - Theme.paddingLarge
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
            }
        }

        model: ChannelTabListModel { }
    }
}

