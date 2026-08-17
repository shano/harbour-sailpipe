import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root
    property string name
    property string thumbnail
    property string url
    property MediaInfo mediaInfo: MediaInfo { id: mediaInfo }
    property alias source: mediaInfo.content

    Component.onCompleted: {
        extractor.downloadExtract(mediaInfo, url);
        comments.model.loadComments(extractor, url);
        DownloadManager.page = url;
        DownloadManager.name = name;
        MediaJunction.controllable = false;
    }

    Component.onDestruction: {
        MediaJunction.controllable = false;
    }

    SilicaListView {
        id: comments
        model: CommentModel {}
        anchors.fill: parent

        VerticalScrollDecorator {}

        onContentYChanged: {
            var pos = contentHeight + originY - height - contentY;
            if ((pos < height) && !comments.model.loading && comments.model.more && comments.model.nextPage) {
                comments.model.loadComments(extractor, url);
            }
        }

        header: Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: header
                //% "YouTube Video"
                title: qsTrId("sailpipe_media-page_header_video")
            }

            Connections {
                target: root

                onStatusChanged: {
                    if (status === PageStatus.Active) {
                        video.state = "hidden"
                        video.parent = video.oldparent;
                        video.state = ""
                    }
                }
            }

            Item {
                width: parent.width
                height: width * (9 / 16)

                VideoPlayer {
                    id: video
                    width: parent.width
                    height: parent.height
                    source: root.source
                    thumbnail: root.thumbnail
                    name: root.name
                    uploader: root.mediaInfo.uploaderName
                }
            }

            ActionBar {
                x: Theme.paddingLarge
                width: parent.width - (2 * Theme.paddingLarge)
                downloadable: (root.source != "")

                onFullscreenPressed: {
                    video.state = "hidden"
                }

                onSharePressed: {
                    shareAction.resources = [{ "type": "text/x-url", "status": root.url.toString() }];
                    shareAction.trigger();
                }

                onDownloadPressed: {
                    DownloadManager.downloadFile(root.source);
                }

                onDownloadCancelPressed: {
                    DownloadManager.cancel();
                }

                onOpenPagePressed: {
                    Qt.openUrlExternally(url)
                }

                ShareAction {
                    id: shareAction
                    mimeType: "text/x-url"
                }
            }

            Label {
                id: videoTitle
                x: Theme.paddingLarge
                width: parent.width - (2 * Theme.paddingLarge)
                color: Theme.highlightColor
                wrapMode: Text.Wrap
                text: root.name
            }

            MediaDetails {
                mediaInfo: root.mediaInfo
            }
        }

        delegate: CommentItem {
            url: root.url
            uploaderAvatar: model.uploaderAvatar
            uploaderName: model.uploaderName
            commentText: model.commentText
            replyCount: model.replyCount
            page: model.page
        }

        ProcessIndicator {
            loading: comments.model.loading
            count: comments.count
            //% "No comments"
            text: qsTrId("sailpipe_comments-no_entries")
            //% "There are no comments here"
            hintText: qsTrId("sailpipe_comments-no_entries_hint")
        }
    }
}

