import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: root
    property string url
    property string uploaderAvatar
    property string uploaderName
    property string commentText
    property int replyCount
    property PageRef page

    Component.onCompleted: {
        comments.model.loadComments(extractor, url);
    }

    SilicaListView {
        id: comments
        model: CommentModel {
            nextPage: root.page
        }
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
                //% "Replies"
                title: qsTrId("sailpipe_replies-page_header")
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
    }
}
