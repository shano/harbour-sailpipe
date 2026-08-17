import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root

    property SearchModel feedModel: SearchModel { id: feedModel }

    Component.onCompleted: {
        refresh();
    }

    function refresh() {
        var urls = Subscriptions.allUrls();
        if (urls.length === 0) {
            feedModel.clear();
            return;
        }
        feedModel.loading = true;
        extractor.getSubscriptionFeed(feedModel, urls);
    }

    SilicaListView {
        id: listView
        model: feedModel
        anchors.fill: parent

        // No pagination for v1 — the feed shows the latest videos per
        // subscribed channel from a single fetch; feedModel.nextPage stays
        // null since getSubscriptionFeed() never sets one.
        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                //% "Manage subscriptions"
                text: qsTrId("sailpipe_subscriptions_page-menu_manage")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("ManageSubscriptionsPage.qml"));
                }
            }
            MenuItem {
                //% "Refresh"
                text: qsTrId("sailpipe_subscriptions_page-menu_refresh")
                onClicked: {
                    root.refresh();
                }
            }
        }

        header: PageHeader {
            //% "Subscriptions"
            title: qsTrId("sailpipe_subscriptions_page-header")
        }

        ProcessIndicator {
            loading: feedModel.loading
            count: listView.count
            //% "No subscriptions"
            text: qsTrId("sailpipe_subscriptions_page-no_entries")
            //% "Subscribe to channels to see their videos here"
            hintText: qsTrId("sailpipe_subscriptions_page-no_entries_hint")
        }

        delegate: SearchDelegate {
            infoType: model.infoType
            thumbnail: model.thumbnail
            name: model.name
            url: model.url
            infoRow: model.infoRow
        }
    }
}
