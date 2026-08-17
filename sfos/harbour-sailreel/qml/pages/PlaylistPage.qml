import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root
    property string name
    property string thumbnail
    property alias url: playlistmodel.url

    Component.onCompleted: {
        playlist.model.search(extractor);
    }

    SilicaListView {
        id: playlist
        model: PlaylistModel {
            id: playlistmodel
        }
        anchors.fill: parent

        VerticalScrollDecorator {}

        onContentYChanged: {
            var pos = contentHeight + originY - height - contentY;
            if ((pos < height) && !playlist.model.loading && playlist.model.more && playlist.model.nextPage) {
                playlist.model.searchMore(extractor);
            }
        }

        header: Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: header
                //% "YouTube Playlist"
                title: qsTrId("sailpipe_playlist_page-header_playlist")
            }

            Label {
                id: playlistTitle
                x: Theme.paddingLarge
                width: parent.width - (2 * Theme.paddingLarge)
                color: Theme.highlightColor
                wrapMode: Text.Wrap
                text: root.name
            }

            PlaylistDetails {
                playlistModel: playlistmodel
            }
        }

        ProcessIndicator {
            loading: playlist.model.loading
            count: playlist.count
            //% "No entries"
            text: qsTrId("sailpipe_playlist-no_entries")
            //% "There are no videos in this playlist"
            hintText: qsTrId("sailpipe_playlist-no_videos")
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
