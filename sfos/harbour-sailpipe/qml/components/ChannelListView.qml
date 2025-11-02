import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

TabItem {
    id: root
    property alias channelmodel: flickable.model
    property alias noitems: processIndicator.text

    anchors.fill: parent
    flickable: flickable

    SilicaListView {
        id: flickable
        anchors.fill: parent

        VerticalScrollDecorator {}

        onContentYChanged: {
            var pos = contentHeight + originY - height - contentY;
            if ((pos < height) && !channelmodel.loading && channelmodel.more && channelmodel.nextPage) {
                channelmodel.searchMore(extractor);
            }
        }

        ProcessIndicator {
            id: processIndicator
            loading: channelmodel.loading
            count: flickable.count
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
