import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.newpipe.extractor 1.0

Page {
    id: root

    SilicaListView {
        id: flickable
        anchors.fill: parent
        model: serviceModel

        VerticalScrollDecorator {}

        header: PageHeader {
            //% "Service"
            title: qsTrId("newpipe_service_page-page_header")
        }

        delegate: BackgroundItem {
            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: model.name
                color: highlighted || (extractor.service === model.service) ? Theme.highlightColor : Theme.primaryColor
            }

            onClicked: {
                extractor.service = model.service
                pageStack.pop();
            }
        }
    }

    ListModel {
        id: serviceModel

        ListElement {
            service: Extractor.YouTubeService
            name: "YouTube"
        }
        ListElement {
            service: Extractor.SoundcloudService
            name: "SoundCloud"
        }
        ListElement {
            service: Extractor.MediaCCCService
            name: "Media.ccc.de"
        }
        ListElement {
            service: Extractor.PeertubeService
            name: "PeerTube"
        }
        ListElement {
            service: Extractor.BandcampService
            name: "Bandcamp"
        }
    }
}
