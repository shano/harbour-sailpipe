import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

TabItem {
    id: root

    property string description
    property int subscriberCount
    property int streamCount
    property bool verified
    property string tags

    anchors.fill: parent
    flickable: flickable

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: content.height + (2 * Theme.paddingLarge)

        VerticalScrollDecorator {}

        Column {
            id: content
            x: Theme.horizontalPageMargin
            y: Theme.paddingLarge
            width: parent.width - (2 * Theme.horizontalPageMargin)
            spacing: Theme.paddingLarge

            AboutKeyValue {
                key: ""
                value: root.description
            }

            GlassItem {
                anchors.horizontalCenter: parent.horizontalCenter
                height: 27
                width: parent.width / 3
                falloffRadius: 0.15
                radius: 0.15
                ratio: 0.0
                color: palette.highlightColor
            }

            AboutKeyValue {
                //% "Subscribers"
                key: qsTrId("newpipe_channel_details-detail_subscribers")
                //% "N/A"
                value: root.subscriberCount >= 0 ? root.subscriberCount : qsTrId("newpipe_channel_details-detail_subscribers_not_applicable")
            }

            AboutKeyValue {
                //% "Verified"
                key: qsTrId("newpipe_channel_details-detail_verified")
                value: root.verified
                         //% "Yes"
                       ? qsTrId("newpipe_channel_details-detail_verified_yes")
                         //% "No"
                       : qsTrId("newpipe_channel_details-detail_verified_no")
            }

            AboutKeyValue {
                //% "Tags"
                key: qsTrId("newpipe_channel_details-detail_tags")
                value: root.tags
            }
        }
    }
}
