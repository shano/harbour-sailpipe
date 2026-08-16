import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: page

    property string searchTerm
    property string screenName
    property int totalitems: 0
    property bool busy: false
    property int displayCount: 0
    readonly property alias pendingSearch: searchTimer.running
    property FilterModel filterModel: FilterModel {
        id: filterModel
        onModelReset: {
            if (!filterValid(searchModel.contentFilter)) {
                searchModel.contentFilter = defaultFilter;
            }
        }
    }
    property SearchModel searchModel: SearchModel {
        id: searchModel
        onContentFilterChanged: searchTimer.restart()
    }

    Component.onCompleted: {
        filterModel.populate(extractor);
    }

    onSearchTermChanged: {
        searchTimer.restart()
    }

    property string errorMessage

    Connections {
        target: extractor
        onServiceChanged: {
            searchModel.clear();
            filterModel.populate(extractor);
            searchTimer.restart()
        }
        onErrorOccurred: {
            page.errorMessage = message;
        }
    }

    Timer {
        id: searchTimer
        interval: 500
        repeat: false
        onTriggered: {
            searchModel.searchTerm = searchTerm;
            searchModel.search(extractor);
        }
    }

    SilicaListView {
        id: listView
        model: searchModel
        anchors.fill: parent

        onCurrentIndexChanged: {
            // This nasty hack prevents the currentIndex being set
            // away from -1
            // This avoids the virtual keyboard disappearing when the
            // search filter is changed
            listView.currentIndex = -1
            //console.log("CurrentIndex: " + currentIndex)
        }

        VerticalScrollDecorator {}

        onContentYChanged: {
            var pos = contentHeight + originY - height - contentY;
            if ((pos < height) && !searchModel.loading && searchModel.more && searchModel.nextPage) {
                searchModel.searchMore(extractor);
            }
        }

        PullDownMenu {
            MenuItem {
                //% "About"
                text: qsTrId("sailpipe_search_page-menu_about")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/AboutPage.qml"));
                }
            }
            MenuItem {
                //% "Settings"
                text: qsTrId("sailpipe_search_page-menu_settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/SettingsPage.qml"));
                }
            }
            MenuItem {
                //% "Service"
                text: qsTrId("sailpipe_search_page-menu_service")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/ServicePage.qml"));
                }
            }
            MenuItem {
                //% "Filter"
                text: qsTrId("sailpipe_search_page-menu_filter")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/FilterPage.qml"), {
                        filterModel: page.filterModel,
                        searchModel: page.searchModel
                    });
                }
                enabled: filterModel.count > 0
            }
        }

        header: Column {
            id: headerColumn
            width: page.width
            height: header.height + searchField.height

            PageHeader {
                id: header
                title: screenName
            }

            SearchField {
                id: searchField
                width: parent.width
                //% "Search %0"
                placeholderText: qsTrId("sailpipe_search_page-search_placeholder").arg(extractor.serviceName)
                // Predictive text actually messes up the clear button so it only
                // works if there's more than one word (weird!), but predictive
                // is likely to be the more useful of the two, so I've left it on
                //inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                inputMethodHints: Qt.ImhNoAutoUppercase

                Binding {
                    target: page
                    property: "searchTerm"
                    value: searchField.text.toLowerCase().trim()
                }
            }
        }

        ProcessIndicator {
            loading: searchModel.loading
            count: listView.count
            //% "No entries"
            text: qsTrId("sailpipe_search_page-search_no_entries")
            //% "Enter some text to search"
            hintText: qsTrId("sailpipe_search_page-search_enter_some_text")
        }

        delegate: SearchDelegate {
            infoType: model.infoType
            thumbnail: model.thumbnail
            name: model.name
            url: model.url
            infoRow: model.infoRow
        }
    }

    Label {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
            bottomMargin: Theme.paddingLarge
        }
        text: page.errorMessage
        visible: text.length > 0
        wrapMode: Text.WordWrap
        color: Theme.highlightColor
        horizontalAlignment: Text.AlignHCenter
    }
}
