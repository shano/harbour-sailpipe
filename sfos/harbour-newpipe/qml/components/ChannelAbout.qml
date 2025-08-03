import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

TabItem {
    id: root

    property string description
    property int subscriberCount
    property int streamCount
    property bool verified

    anchors.fill: parent
    flickable: flickable

    SilicaFlickable {
        id: flickable
        anchors.fill: parent

        VerticalScrollDecorator {}

        Column {
            x: Theme.horizontalPageMargin
            y: Theme.paddingLarge
            width: parent.width - (2 * Theme.horizontalPageMargin)
            spacing: Theme.paddingLarge

            AboutKeyValue {
                //% "Description"
                key: qsTrId("newpipe-channel_details-detail_description")
                value: root.description
            }

            AboutKeyValue {
                //% "Subscribers"
                key: qsTrId("newpipe-channel_details-detail_subscribers")
                value: root.subscriberCount
            }

            AboutKeyValue {
                //% "Items"
                key: qsTrId("newpipe-channel_details-detail_streams")
                value: root.streamCount
            }

            AboutKeyValue {
                //% "Verified"
                key: qsTrId("newpipe-channel_details-detail_verified")
                value: root.verified
                         //% "Yes"
                       ? qsTrId("newpipe-channel_details-detail_verified_yes")
                         //% "No"
                       : qsTrId("newpipe-channel_details-detail_verified_no")
            }
        }
    }
}
