import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.sailpipe.extractor 1.0

Page {
    id: settingsPage

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                //% "Settings"
                title: qsTrId("sailpipe_settings-title")
            }

            SectionHeader {
                //% "yt-dlp (YouTube backend)"
                text: qsTrId("sailpipe_settings-section_ytdlp")
            }

            Label {
                //% "YouTube extraction and downloads use yt-dlp, a separate open-source tool that needs to be installed and kept up to date on this device."
                text: qsTrId("sailpipe_settings-ytdlp_description")
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
                //% "Status"
                label: qsTrId("sailpipe_settings-ytdlp_status")
                value: {
                    switch (YtDlp.status) {
                    case YtDlpManager.NotInstalled:
                        //% "Not installed"
                        return qsTrId("sailpipe_settings-ytdlp_status_not_installed");
                    case YtDlpManager.Installed:
                        //% "Installed"
                        return qsTrId("sailpipe_settings-ytdlp_status_installed");
                    case YtDlpManager.CheckingForUpdate:
                        //% "Checking for updates…"
                        return qsTrId("sailpipe_settings-ytdlp_status_checking");
                    case YtDlpManager.Downloading:
                        //% "Downloading…"
                        return qsTrId("sailpipe_settings-ytdlp_status_downloading");
                    default:
                        //% "Error"
                        return qsTrId("sailpipe_settings-ytdlp_status_error");
                    }
                }
            }

            InfoRow {
                //% "Installed version"
                label: qsTrId("sailpipe_settings-ytdlp_installed_version")
                value: YtDlp.installedVersion.length > 0 ? YtDlp.installedVersion : "—"
            }

            InfoRow {
                //% "Latest version"
                label: qsTrId("sailpipe_settings-ytdlp_latest_version")
                value: YtDlp.latestVersion.length > 0 ? YtDlp.latestVersion : "—"
            }

            ProgressBar {
                width: parent.width
                x: Theme.horizontalPageMargin
                visible: YtDlp.status === YtDlpManager.Downloading
                value: YtDlp.progress
                minimumValue: 0.0
                maximumValue: 1.0
            }

            Row {
                spacing: Theme.paddingLarge
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    //% "Check for Updates"
                    text: qsTrId("sailpipe_settings-ytdlp_check_updates")
                    enabled: YtDlp.status !== YtDlpManager.Downloading && YtDlp.status !== YtDlpManager.CheckingForUpdate
                    onClicked: YtDlp.checkForUpdate()
                }

                Button {
                    //% "Install"
                    text: YtDlp.status === YtDlpManager.NotInstalled
                        ? qsTrId("sailpipe_settings-ytdlp_install")
                        : qsTrId("sailpipe_settings-ytdlp_update")
                    enabled: YtDlp.status !== YtDlpManager.Downloading
                    onClicked: {
                        if (YtDlp.status === YtDlpManager.NotInstalled) {
                            YtDlp.install();
                        } else {
                            YtDlp.update();
                        }
                    }
                }
            }

            Connections {
                target: YtDlp
                onErrorOccurred: {
                    //% "yt-dlp error: %1"
                    banner.text = qsTrId("sailpipe_settings-ytdlp_error").arg(message);
                    banner.show();
                }
            }

            Label {
                id: banner
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.errorColor
                visible: text.length > 0
                function show() { visible = true; }
                anchors {
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                    left: parent.left
                    right: parent.right
                }
            }
        }
    }
}
