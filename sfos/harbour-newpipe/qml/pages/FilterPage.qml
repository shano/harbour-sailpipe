import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

Page {
    id: root
    property alias filterModel: flickable.model
    property SearchModel searchModel

    SilicaListView {
        id: flickable
        anchors.fill: parent

        VerticalScrollDecorator {}

        header: PageHeader {
            //% "Search Filter"
            title: qsTrId("newpipe_filter_page-page_header")
        }

        delegate: BackgroundItem {
            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: model.name
                color: highlighted || (searchModel.contentFilter === model.filter) ? Theme.highlightColor : Theme.primaryColor
            }

            onClicked: {
                root.searchModel.contentFilter = model.filter;
                pageStack.pop();
            }
        }
    }
}
