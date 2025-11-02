import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0
import "../components"

Page {
    id: aboutPage
    SilicaFlickable {
        width: parent.width
        height: parent.height
        interactive: true

        anchors.fill: parent
        contentHeight: aboutColumn.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: aboutColumn
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                //% "About SailPipe"
                title: qsTrId("sailpipe_about-title")
            }

            Image {
                anchors.topMargin: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                source  : Qt.resolvedUrl("image://sailpipe/sailpipe-title")
            }

            Label {
                //% "Video and music streaming and downloading"
                text: qsTrId("sailpipe_about-description")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                anchors {
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    left: parent.left
                    right: parent.right
                }
            }

            InfoRow {
                //% "Version"
                label: qsTrId("sailpipe_about-version")
                value: sailpipeVersion
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Maintainer"
                label: qsTrId("sailpipe_about-maintainer")
                value: "David Llewellyn-Jones"
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Licence"
                label: qsTrId("sailpipe_about-licence")
                value: "GPLv3.0"
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            Label {
                //% "Please respect the copyright of all files downloaded using this software"
                text: qsTrId("sailpipe_about-respect_copyright")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                anchors {
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    left: parent.left
                    right: parent.right
                }
            }

            Label {
                //% "A user interface to the NewPipe Extractor for streaming and downloading video and music from multiple online services, including YouTube, SoundCloud, Media.cc.de and Bandcamp."
                text: qsTrId("sailpipe_about-summary")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                anchors {
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    left: parent.left
                    right: parent.right
                }
            }

            SectionHeader {
                //% "About NewPipe Extractor"
                text: qsTrId("sailpipe_about-newpipe_extractor")
            }

            Label {
                //% "A library for extracting things from streaming sites"
                text: qsTrId("sailpipe_about-newpipe_extractor_description")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeSmall
                anchors {
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    left: parent.left
                    right: parent.right
                }
            }

            InfoRow {
                //% "Version"
                label: qsTrId("sailpipe_about-newpipe_extractor_version")
                value: "0.24.5"
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Maintainer"
                label: qsTrId("sailpipe_about-newpipe_extractor_maintainer")
                value: "TeamNewPipe"
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Licence"
                label: qsTrId("sailpipe_about-newpipie_extractor_licence")
                value: "GPLv3.0"
                midlineRatio: 0.3
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            SectionHeader {
                //% "Contributors"
                text: qsTrId("sailpipe_about-subtitle_contributors")
            }

            InfoRow {
                //% "Belarusian"
                label: qsTrId("sailpipe_about-belarusian_translation")
                value: "Toha"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Estonian"
                label: qsTrId("sailpipe_about-estonian_translation")
                value: "Priit Jõerüüt"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Finnish"
                label: qsTrId("sailpipe_about-finnish_translation")
                value: "Elmeri Länsiharju"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "French"
                label: qsTrId("sailpipe_about-french_translation")
                value: "Robin Grenet"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "German"
                label: qsTrId("sailpipe_about-german_translation")
                value: "thigg, Leif-Jöran Olsson"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Italian"
                label: qsTrId("sailpipe_about-italian_translation")
                value: "legacychimera247"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Polish"
                label: qsTrId("sailpipe_about-polish_translation")
                value: "Paweł Koszała"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Russian"
                label: qsTrId("sailpipe_about-russian_translation")
                value: "RoundedRectangle"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Swedish"
                label: qsTrId("sailpipe_about-swedish_translation")
                value: "Leif-Jöran Olsson"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Turkish"
                label: qsTrId("sailpipe_about-turkish_translation")
                value: "Oğuz Ersen"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            InfoRow {
                //% "Translation platform"
                label: qsTrId("sailpipe_about-tralsation_platform")
                value: "Weblate"
                midlineRatio: 0.5
                midlineMin: Theme.fontSizeSmall * 5
                midlineMax: Theme.fontSizeSmall * 10
            }

            SectionHeader {
                //% "Links"
                text: qsTrId("sailpipe_about-subtitle_links")
            }

            Row {
                spacing: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    id: connect
                    //% "Website"
                    text: qsTrId("sailpipe_about-website")
                    enabled: true
                    onClicked: Qt.openUrlExternally("http://www.flypig.co.uk/sailpipe")
                }
                Button {
                    id : disconnect
                    //% "Email"
                    text: qsTrId("sailpipe_about-email")
                    enabled: true
                    onClicked: Qt.openUrlExternally("mailto:david@flypig.co.uk")
                }
            }
        }
    }
}
