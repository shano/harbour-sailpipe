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
                //% "About SailReel"
                title: qsTrId("sailpipe_about-title_about_sailpipe")
            }

            Label {
                anchors.topMargin: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Theme.fontSizeHuge
                font.bold: true
                color: Theme.highlightColor
                text: "SailReel"
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
                //% "A user interface to yt-dlp for streaming and downloading video from YouTube."
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
                //% "yt-dlp"
                text: qsTrId("sailpipe_about-section_ytdlp")
            }

            Label {
                //% "YouTube extraction and downloads use yt-dlp, a separate open-source tool this app downloads and manages itself. Manage installs and updates from Settings."
                text: qsTrId("sailpipe_about-ytdlp_description")
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
                //% "Installed version"
                label: qsTrId("sailpipe_about-ytdlp_version")
                value: YtDlp.installedVersion.length > 0 ? YtDlp.installedVersion : "—"
            }

            SectionHeader {
                //% "Contributors"
                text: qsTrId("sailpipe_about-section_contributors")
            }

            Label {
                //% "The amazing team of SailReel contributors"
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
                //% "Project poet"
                label: qsTrId("sailpipe_about-poet")
                value: "Leif-Jöran Olsson"
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
                    onClicked: Qt.openUrlExternally("https://github.com/shano/harbour-sailreel")
                }
                Button {
                    id : disconnect
                    //% "Report an Issue"
                    text: qsTrId("sailpipe_about-report_issue")
                    enabled: true
                    onClicked: Qt.openUrlExternally("https://github.com/shano/harbour-sailreel/issues")
                }
            }
        }
    }
}
