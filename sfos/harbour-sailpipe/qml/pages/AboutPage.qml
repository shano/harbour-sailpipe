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
                title: qsTrId("sailpipe_about-title_about_sailpipe")
            }

            Image {
                anchors.topMargin: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                source  : Qt.resolvedUrl("image://sailpipe/sailpipe-title")
            }

            SectionHeader {
                //% "General"
                text: qsTrId("sailpipe_about-section_general")
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
            }

            InfoRow {
                //% "Maintainer"
                label: qsTrId("sailpipe_about-maintainer")
                value: "David Llewellyn-Jones"
            }

            InfoRow {
                //% "Licence"
                label: qsTrId("sailpipe_about-licence")
                value: "GPLv3.0"
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
                //% "A user interface to the NewPipe Extractor for streaming and downloading video and music from multiple online services, including YouTube, SoundCloud, Media.ccc.de and Bandcamp."
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
                //% "NewPipe Extractor"
                text: qsTrId("sailpipe_about-section_newpipe_extractor")
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
            }

            InfoRow {
                //% "Maintainer"
                label: qsTrId("sailpipe_about-newpipe_extractor_maintainer")
                value: "TeamNewPipe"
            }

            InfoRow {
                //% "Licence"
                label: qsTrId("sailpipe_about-newpipie_extractor_licence")
                value: "GPLv3.0"
            }

            SectionHeader {
                //% "Contributors"
                text: qsTrId("sailpipe_about-section_contributors")
            }

            Label {
                //% "The amazing team of SailPipe contributors"
                text: qsTrId("sailpipe_about-contributors_description")
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
                //% "Sailing-to-Coffee"
                label: qsTrId("sailpipe_about-sailing_to_coffee")
                value: "Thilo"
            }

            InfoRow {
                //% "Opal modules"
                label: qsTrId("sailpipe_about-opal")
                value: "Mirian Margiani"
            }

            InfoRow {
                //% "Testing"
                label: qsTrId("sailpipe_about-testing")
                value: "ExPLIT"
            }

            InfoRow {
                //% "Belarusian"
                label: qsTrId("sailpipe_about-belarusian_translation")
                value: "Toha"
            }

            InfoRow {
                //% "Estonian"
                label: qsTrId("sailpipe_about-estonian_translation")
                value: "Priit Jõerüüt"
            }

            InfoRow {
                //% "Finnish"
                label: qsTrId("sailpipe_about-finnish_translation")
                value: "Elmeri Länsiharju"
            }

            InfoRow {
                //% "French"
                label: qsTrId("sailpipe_about-french_translation")
                value: "Robin Grenet"
            }

            InfoRow {
                //% "German"
                label: qsTrId("sailpipe_about-german_translation")
                value: "thigg, Leif-Jöran Olsson"
            }

            InfoRow {
                //% "Italian"
                label: qsTrId("sailpipe_about-italian_translation")
                value: "legacychimera247"
            }

            InfoRow {
                //% "Polish"
                label: qsTrId("sailpipe_about-polish_translation")
                value: "Paweł Koszała"
            }

            InfoRow {
                //% "Russian"
                label: qsTrId("sailpipe_about-russian_translation")
                value: "RoundedRectangle"
            }

            InfoRow {
                //% "Swedish"
                label: qsTrId("sailpipe_about-swedish_translation")
                value: "Leif-Jöran Olsson"
            }

            InfoRow {
                //% "Turkish"
                label: qsTrId("sailpipe_about-turkish_translation")
                value: "Oğuz Ersen"
            }

            InfoRow {
                //% "Romanian"
                label: qsTrId("sailpipe_about-romanian_translation")
                value: "dumol"
            }

            InfoRow {
                //% "Translation platform"
                label: qsTrId("sailpipe_about-tralsation_platform")
                value: "Weblate"
            }

            SectionHeader {
                //% "Links"
                text: qsTrId("sailpipe_about-section_links")
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
