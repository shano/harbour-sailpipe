import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: Subscriptions

        VerticalScrollDecorator {}

        header: PageHeader {
            //% "Manage Subscriptions"
            title: qsTrId("sailpipe_manage_subscriptions_page-header")
        }

        ViewPlaceholder {
            enabled: listView.count === 0
            //% "No subscriptions"
            text: qsTrId("sailpipe_subscriptions_page-no_entries")
            //% "Subscribe to channels to see them here"
            hintText: qsTrId("sailpipe_manage_subscriptions_page-no_entries_hint")
        }

        delegate: ListItem {
            id: delegateItem
            contentHeight: Theme.itemSizeMedium

            SearchThumbnail {
                id: thumbnail
                infoType: SearchItem.Channel
                source: model.thumbnail
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }

            Label {
                anchors {
                    left: thumbnail.right
                    leftMargin: Theme.paddingMedium
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                text: model.name
                truncationMode: TruncationMode.Fade
            }

            onClicked: {
                pageStack.push(Qt.resolvedUrl("ChannelPage.qml"), {
                    name: model.name,
                    thumbnail: model.thumbnail,
                    url: model.url
                });
            }

            menu: ContextMenu {
                MenuItem {
                    //% "Remove"
                    text: qsTrId("sailpipe_manage_subscriptions_page-remove")
                    onClicked: {
                        var row = index;
                        //% "Removing"
                        delegateItem.remorseAction(qsTrId("sailpipe_manage_subscriptions_page-removing"), function() {
                            Subscriptions.removeAt(row);
                        });
                    }
                }
            }
        }
    }
}
