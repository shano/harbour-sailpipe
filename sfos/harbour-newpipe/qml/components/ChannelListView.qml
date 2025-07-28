import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

TabItem {
    id: root
    property alias channelmodel: flickable.model
    property alias noitems: placeholder.text

    anchors.fill: parent
    flickable: flickable

    SilicaListView {
        id: flickable
        anchors.fill: parent

        VerticalScrollDecorator {}

        onContentYChanged: {
            var pos = contentHeight + originY - height - contentY;
            if ((pos < height) && !root.channelmodel.loading && root.channelmodel.more && root.channelmodel.nextPage) {
                root.channelmodel.searchMore(extractor);
            }
        }

        ViewPlaceholder {
            id: placeholder
            enabled: flickable.count === 0
            textFormat: Text.RichText
            text: "No videos"
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
