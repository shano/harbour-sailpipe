# yt-dlp Backend for YouTube Extraction Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace NewPipeExtractor with yt-dlp as the extraction and download backend for the YouTube service only, and add UI to install/update the yt-dlp binary.

**Architecture:** `Extractor` (`sfos/harbour-sailpipe/src/extractor.cpp`) branches per-call on `m_service`: YouTube routes to a new `YtDlpBackend`, all other services keep using the existing GraalVM `Invoke` path. `YtDlpBackend` shells out to an app-managed `yt-dlp` binary via `QProcess`, parses its JSON output, and translates it into the exact NewPipeExtractor JSON shape the existing Qt models (`SearchItem`, `ChannelInfo`, `PlaylistModel`, `CommentItem`, `MediaInfo`) already expect — so those models and all QML pages are untouched. A new `YtDlpManager` singleton handles downloading/updating the yt-dlp binary itself, exposed to a new `SettingsPage.qml`.

**Tech Stack:** C++17, Qt5 (Core, Network, Qml, Quick), Sailfish Silica, QProcess, QNetworkAccessManager, Qt Test (new — no test infrastructure exists in this project today).

## Global Constraints

- Full design: `docs/superpowers/specs/2026-08-10-ytdlp-youtube-backend-design.md`. Read it before starting — this plan implements it task-by-task.
- Scope is YouTube only. SoundCloud, MediaCCC, PeerTube, Bandcamp keep using GraalVM/NewPipeExtractor — do not touch `java/`, `cpp/`, the GraalVM build, or `Invoke`/`invoke.cpp` for non-YouTube paths.
- **No build environment is reachable from this session** (no Sailfish Platform SDK, no `mb2`/`sfdk`, no Qt5 dev packages, no docker/podman). Every step that requires compiling or running the app is marked **MANUAL VERIFICATION** and is something *you* run in your own Sailfish SDK/emulator/device — not something executed as part of this plan's automated steps. Steps not marked that way (reading code, grepping, writing files) can run in this session.
- No mocking internals; only mock/stub at system boundaries (the `yt-dlp` process, the GitHub API, the filesystem) — per project test conventions. `YtDlpTranslate`'s pure JSON-mapping functions take real captured yt-dlp JSON as input in tests, not mocks.
- Match existing code style exactly: 2-space indent, `m_` prefix for members, `Q_PROPERTY`/signal-per-setter pattern, singleton `instantiate()`/`getInstance()`/`provider()` pattern (see `DownloadManager`, `MediaJunction`).
- yt-dlp binary: standalone, self-contained releases confirmed via GitHub API (tag `2026.07.04`): `yt-dlp_linux_aarch64` (raw executable) and `yt-dlp_linux_armv7l.zip`. No Python runtime dependency. Stored at `~/.local/share/harbour-sailpipe/yt-dlp/yt-dlp` (app-private, not on `PATH`).
- Sailjail profile already grants `Internet` — no profile change needed for GitHub API calls or yt-dlp's own network access.

---

### Task 1: `YtDlpManager` — binary lifecycle (install/update)

**Files:**
- Create: `sfos/harbour-sailpipe/src/ytdlpmanager.h`
- Create: `sfos/harbour-sailpipe/src/ytdlpmanager.cpp`

**Interfaces:**
- Produces: `YtDlpManager::binaryPath()` (static, `QString`) — the absolute path where the yt-dlp binary lives, whether or not it's installed yet. Used by `YtDlpProcess` (Task 4) and `DownloadManager` (Task 11).
- Produces: `YtDlpManager` QObject singleton with `Q_PROPERTY status`, `Q_PROPERTY installedVersion`, slots `checkForUpdate()`, `install()`, `update()`, signals `statusChanged()`, `installedVersionChanged()`, `progressChanged(float)`, `errorOccurred(QString)`. Consumed by `SettingsPage.qml` (Task 2).

- [ ] **Step 1: Write `ytdlpmanager.h`**

```cpp
#ifndef YTDLPMANAGER_H
#define YTDLPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QQmlEngine;
class QJSEngine;

class YtDlpManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY(Status status READ status NOTIFY statusChanged)
  Q_PROPERTY(QString installedVersion READ installedVersion NOTIFY installedVersionChanged)
  Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestVersionChanged)
  Q_PROPERTY(float progress READ progress NOTIFY progressChanged)

public:
  enum Status {
    NotInstalled,
    Installed,
    CheckingForUpdate,
    Downloading,
    Error,
  };
  Q_ENUM(Status)

  explicit YtDlpManager(QObject *parent = nullptr);

  static void instantiate(QObject* parent = nullptr);
  static YtDlpManager& getInstance();
  static QObject* provider(QQmlEngine* engine, QJSEngine* scriptEngine);

  static QString binaryPath();
  static QString releaseAssetName();

  Status status() const;
  QString installedVersion() const;
  QString latestVersion() const;
  float progress() const;

public slots:
  void refreshInstalledVersion();
  void checkForUpdate();
  void install();
  void update();

signals:
  void statusChanged();
  void installedVersionChanged();
  void latestVersionChanged();
  void progressChanged();
  void errorOccurred(QString message);

private slots:
  void onLatestReleaseReply();
  void onAssetDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onAssetDownloadFinished();

private:
  void setStatus(Status status);
  void setInstalledVersion(QString const& version);
  void setLatestVersion(QString const& version);
  void setProgress(float progress);
  void startDownload(QString const& downloadUrl);

private:
  static YtDlpManager* m_instance;
  QNetworkAccessManager* m_manager;
  QNetworkReply* m_activeReply;
  Status m_status;
  QString m_installedVersion;
  QString m_latestVersion;
  float m_progress;
};

#endif // YTDLPMANAGER_H
```

- [ ] **Step 2: Write `ytdlpmanager.cpp`**

```cpp
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSysInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QNetworkRequest>
#include <QUrlQuery>

#include "ytdlpmanager.h"

#define GITHUB_LATEST_RELEASE_URL "https://api.github.com/repos/yt-dlp/yt-dlp/releases/latest"

YtDlpManager* YtDlpManager::m_instance = nullptr;

YtDlpManager::YtDlpManager(QObject *parent)
  : QObject(parent)
  , m_manager(new QNetworkAccessManager(this))
  , m_activeReply(nullptr)
  , m_status(NotInstalled)
  , m_installedVersion()
  , m_latestVersion()
  , m_progress(0.0)
{
  refreshInstalledVersion();
}

void YtDlpManager::instantiate(QObject* parent)
{
  if (m_instance == nullptr) {
    m_instance = new YtDlpManager(parent);
  }
}

YtDlpManager& YtDlpManager::getInstance()
{
  return *m_instance;
}

QObject* YtDlpManager::provider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
  Q_UNUSED(engine)
  Q_UNUSED(scriptEngine)

  return m_instance;
}

QString YtDlpManager::binaryPath()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return QString("%1/yt-dlp/yt-dlp").arg(dataDir);
}

QString YtDlpManager::releaseAssetName()
{
  QString arch = QSysInfo::currentCpuArchitecture();
  if (arch == QLatin1String("arm64")) {
    return QStringLiteral("yt-dlp_linux_aarch64");
  }
  // armv7hl devices report "arm" from QSysInfo; only the zipped asset
  // exists upstream for this architecture. Extraction is handled by the
  // caller (install()) when the asset name ends in .zip.
  return QStringLiteral("yt-dlp_linux_armv7l.zip");
}

YtDlpManager::Status YtDlpManager::status() const
{
  return m_status;
}

QString YtDlpManager::installedVersion() const
{
  return m_installedVersion;
}

QString YtDlpManager::latestVersion() const
{
  return m_latestVersion;
}

float YtDlpManager::progress() const
{
  return m_progress;
}

void YtDlpManager::setStatus(Status status)
{
  if (m_status != status) {
    m_status = status;
    emit statusChanged();
  }
}

void YtDlpManager::setInstalledVersion(QString const& version)
{
  if (m_installedVersion != version) {
    m_installedVersion = version;
    emit installedVersionChanged();
  }
}

void YtDlpManager::setLatestVersion(QString const& version)
{
  if (m_latestVersion != version) {
    m_latestVersion = version;
    emit latestVersionChanged();
  }
}

void YtDlpManager::setProgress(float progress)
{
  if (m_progress != progress) {
    m_progress = progress;
    emit progressChanged();
  }
}

void YtDlpManager::refreshInstalledVersion()
{
  QFileInfo info(binaryPath());
  if (!info.exists() || !info.isExecutable()) {
    setInstalledVersion(QString());
    setStatus(NotInstalled);
    return;
  }

  QProcess process;
  process.start(binaryPath(), QStringList() << QStringLiteral("--version"));
  if (process.waitForFinished(5000) && (process.exitCode() == 0)) {
    QString version = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    setInstalledVersion(version);
    setStatus(Installed);
  }
  else {
    setInstalledVersion(QString());
    setStatus(Error);
  }
}

void YtDlpManager::checkForUpdate()
{
  setStatus(CheckingForUpdate);

  QNetworkRequest request{QUrl(QStringLiteral(GITHUB_LATEST_RELEASE_URL))};
  request.setRawHeader("Accept", "application/vnd.github+json");
  request.setRawHeader("User-Agent", "harbour-sailpipe");

  m_activeReply = m_manager->get(request);
  connect(m_activeReply, &QNetworkReply::finished, this, &YtDlpManager::onLatestReleaseReply);
}

void YtDlpManager::onLatestReleaseReply()
{
  QNetworkReply* reply = m_activeReply;
  m_activeReply = nullptr;

  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
    reply->deleteLater();
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
  QString tag = doc.object()["tag_name"].toString();
  setLatestVersion(tag);
  setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);

  reply->deleteLater();
}

void YtDlpManager::install()
{
  QString assetName = releaseAssetName();
  QString downloadUrl = QString("https://github.com/yt-dlp/yt-dlp/releases/latest/download/%1").arg(assetName);
  startDownload(downloadUrl);
}

void YtDlpManager::update()
{
  install();
}

void YtDlpManager::startDownload(QString const& downloadUrl)
{
  setStatus(Downloading);
  setProgress(0.0);

  QNetworkRequest request{QUrl(downloadUrl)};
  request.setMaximumRedirectsAllowed(10);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  m_activeReply = m_manager->get(request);
  connect(m_activeReply, &QNetworkReply::downloadProgress, this, &YtDlpManager::onAssetDownloadProgress);
  connect(m_activeReply, &QNetworkReply::finished, this, &YtDlpManager::onAssetDownloadFinished);
}

void YtDlpManager::onAssetDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if (bytesTotal > 0) {
    setProgress(static_cast<float>(bytesReceived) / static_cast<float>(bytesTotal));
  }
}

void YtDlpManager::onAssetDownloadFinished()
{
  QNetworkReply* reply = m_activeReply;
  m_activeReply = nullptr;

  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred(reply->errorString());
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
    reply->deleteLater();
    return;
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  QString path = binaryPath();
  QDir().mkpath(QFileInfo(path).absolutePath());

  QString assetName = releaseAssetName();
  bool ok = false;
  if (assetName.endsWith(QStringLiteral(".zip"))) {
    // armv7l asset ships zipped. Extraction requires the `unzip`
    // command-line tool to be present on-device.
    QString tmpZip = QString("%1.zip").arg(path);
    QFile zipFile(tmpZip);
    if (zipFile.open(QIODevice::WriteOnly)) {
      zipFile.write(data);
      zipFile.close();

      QProcess unzip;
      unzip.setWorkingDirectory(QFileInfo(path).absolutePath());
      unzip.start(QStringLiteral("unzip"), QStringList() << QStringLiteral("-o") << tmpZip);
      ok = unzip.waitForFinished(30000) && (unzip.exitCode() == 0);
      QFile::remove(tmpZip);

      if (ok) {
        // The zip contains a single file; find it and rename to binaryPath().
        QDir dir(QFileInfo(path).absolutePath());
        QStringList entries = dir.entryList(QDir::Files);
        for (QString const& entry : entries) {
          if (entry != QFileInfo(path).fileName() && !entry.endsWith(QStringLiteral(".zip"))) {
            QFile::remove(path);
            ok = QFile::rename(dir.filePath(entry), path);
            break;
          }
        }
      }
    }
  }
  else {
    QFile outFile(path);
    ok = outFile.open(QIODevice::WriteOnly);
    if (ok) {
      outFile.write(data);
      outFile.close();
    }
  }

  if (ok) {
    QFile::setPermissions(path, QFile::permissions(path)
      | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    refreshInstalledVersion();
  }
  else {
    emit errorOccurred(QStringLiteral("Failed to install yt-dlp binary"));
    setStatus(m_installedVersion.isEmpty() ? NotInstalled : Installed);
  }
}
```

- [ ] **Step 3: MANUAL VERIFICATION — compile check**

Not runnable in this session (no Qt5/Sailfish SDK). Once you have `ytdlpmanager.h`/`.cpp` wired into `CMakeLists.txt` (Task 12), build with your Sailfish SDK (`mb2 build` or equivalent) and confirm no compile errors in these two files.

- [ ] **Step 4: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpmanager.h sfos/harbour-sailpipe/src/ytdlpmanager.cpp
git commit -m "Add YtDlpManager for yt-dlp binary install/update"
```

---

### Task 2: Settings UI for yt-dlp install/update

**Files:**
- Create: `sfos/harbour-sailpipe/qml/pages/SettingsPage.qml`
- Modify: `sfos/harbour-sailpipe/qml/pages/SearchPage.qml:78-92` (add pulley menu entry)
- Modify: `sfos/harbour-sailpipe/qml/pages/AboutPage.qml:98-133` (add yt-dlp version note)
- Modify: `sfos/harbour-sailpipe/qml/pages/ServicePage.qml:28-31` (warn if yt-dlp isn't installed when YouTube is selected)

**Interfaces:**
- Consumes: `YtDlpManager` singleton registered as QML type `YtDlp` in module `harbour.sailpipe.extractor` (registered in Task 12) — properties `status` (enum `YtDlpManager.NotInstalled|Installed|CheckingForUpdate|Downloading|Error`), `installedVersion`, `latestVersion`, `progress`; slots `checkForUpdate()`, `install()`, `update()`; signal `errorOccurred(string)`.

- [ ] **Step 1: Write `SettingsPage.qml`**

```qml
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
```

- [ ] **Step 2: Add pulley menu entry in `SearchPage.qml`**

Modify the `PullDownMenu` block at `sfos/harbour-sailpipe/qml/pages/SearchPage.qml:78-104` — add a new `MenuItem` alongside the existing "About"/"Service"/"Filter" entries:

```qml
        PullDownMenu {
            MenuItem {
                //% "About"
                text: qsTrId("sailpipe_search_page-menu_about")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/AboutPage.qml"));
                }
            }
            MenuItem {
                //% "Settings"
                text: qsTrId("sailpipe_search_page-menu_settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/SettingsPage.qml"));
                }
            }
            MenuItem {
                //% "Service"
                text: qsTrId("sailpipe_search_page-menu_service")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/ServicePage.qml"));
                }
            }
            MenuItem {
                //% "Filter"
                text: qsTrId("sailpipe_search_page-menu_filter")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/FilterPage.qml"), {
                        filterModel: page.filterModel,
                        searchModel: page.searchModel
                    });
                }
                enabled: filterModel.count > 0
            }
        }
```

- [ ] **Step 3: Add yt-dlp note in `AboutPage.qml`**

Modify `sfos/harbour-sailpipe/qml/pages/AboutPage.qml` — insert a new section directly after the existing "NewPipe Extractor" section (after line 132, before the "Contributors" `SectionHeader` at line 134):

```qml
            SectionHeader {
                //% "yt-dlp"
                text: qsTrId("sailpipe_about-section_ytdlp")
            }

            Label {
                //% "YouTube extraction and downloads use yt-dlp instead of NewPipe Extractor, since YouTube changes frequently enough that yt-dlp's faster release cadence keeps things working. Manage installs and updates from Settings."
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
```

Also add the import at the top of `AboutPage.qml:1-4` (it already imports `harbour.sailpipe.extractor 1.0` at line 3, where `YtDlp` will be registered — no new import line needed).

- [ ] **Step 4: Warn when YouTube is selected without yt-dlp installed**

Per the design doc's error-handling section: "if yt-dlp isn't installed and the user selects YouTube, surface a clear prompt directing them to Settings rather than failing silently." Modify `sfos/harbour-sailpipe/qml/pages/ServicePage.qml:28-31`:

```qml
            onClicked: {
                extractor.service = model.service
                if ((model.service === Extractor.YouTubeService) && (YtDlp.status === YtDlpManager.NotInstalled)) {
                    pageStack.replace(Qt.resolvedUrl("SettingsPage.qml"));
                    //% "Install yt-dlp to use YouTube"
                    notice.show(qsTrId("sailpipe_service_page-ytdlp_required_notice"));
                }
                else {
                    pageStack.pop();
                }
            }
```

This needs a `Notice` element available in scope — add it to the `Page` root in `ServicePage.qml` (after the `SilicaListView` block, before the `ListModel` at line 35):

```qml
    Notice {
        id: notice
    }
```

- [ ] **Step 5: MANUAL VERIFICATION**

Build and run on-device or in the Sailfish emulator; open the pulley menu on the search page, confirm "Settings" entry appears and navigates to `SettingsPage`; confirm `AboutPage` shows the new yt-dlp section without a QML binding error (it will show "—" for version until Task 1/12 are wired up and yt-dlp isn't installed yet — that's expected at this point). Confirm that selecting YouTube on `ServicePage` before installing yt-dlp navigates to Settings with a notice instead of silently returning to a search page that will fail.

- [ ] **Step 6: Commit**

```bash
git add sfos/harbour-sailpipe/qml/pages/SettingsPage.qml sfos/harbour-sailpipe/qml/pages/SearchPage.qml \
        sfos/harbour-sailpipe/qml/pages/AboutPage.qml sfos/harbour-sailpipe/qml/pages/ServicePage.qml
git commit -m "Add Settings page for yt-dlp install/update UI"
```

---

### Task 3: `YtDlpTranslate` — pure JSON mapping functions + tests

This is the core translation layer: pure functions, no process/network I/O, so it's the one part of this feature that's realistically unit-testable. Sets up Qt Test infrastructure for the project (none exists today).

**Files:**
- Create: `sfos/harbour-sailpipe/src/ytdlptranslate.h`
- Create: `sfos/harbour-sailpipe/src/ytdlptranslate.cpp`
- Create: `sfos/harbour-sailpipe/tests/CMakeLists.txt`
- Create: `sfos/harbour-sailpipe/tests/testytdlptranslate.cpp`
- Create: `sfos/harbour-sailpipe/tests/testdata/ytdlp_search_entry.json`
- Create: `sfos/harbour-sailpipe/tests/testdata/ytdlp_video_info.json`
- Create: `sfos/harbour-sailpipe/tests/testdata/ytdlp_comment.json`
- Modify: `sfos/harbour-sailpipe/CMakeLists.txt` (add `add_subdirectory(tests)`, `enable_testing()`)

**Interfaces:**
- Produces: `YtDlpTranslate::streamItem`, `::searchResults`, `::mediaInfo`, `::commentItem`, `::commentResults`, `::channelInfo`, `::playlistExtra` — all `QJsonObject`, all pure. Consumed by `YtDlpBackend` (Tasks 5-9).

- [ ] **Step 1: Capture real yt-dlp fixture data**

These fixtures are real yt-dlp output, captured once and checked in — not fabricated. Run on any machine with yt-dlp installed (this sandbox has one at `~/.local/bin/yt-dlp`):

```bash
yt-dlp -J --flat-playlist "ytsearch1:sailfish os" | python3 -c "import json,sys; d=json.load(sys.stdin); print(json.dumps(d['entries'][0], indent=2))" > /tmp/entry.json
```

Take the single-entry JSON this produces, strip anything that looks like a session-specific token (there shouldn't be any in flat-playlist output), and save as `sfos/harbour-sailpipe/tests/testdata/ytdlp_search_entry.json`.

Similarly:
```bash
yt-dlp -J "https://www.youtube.com/watch?v=dQw4w9WgXcQ" > sfos/harbour-sailpipe/tests/testdata/ytdlp_video_info.json
```

For the comment fixture, since capturing one from `--write-comments` output requires a video with comments enabled and takes longer to run, hand-write a minimal realistic one matching yt-dlp's documented comment schema instead:

```json
{
  "text": "Great video, thanks for sharing!",
  "author": "Someone",
  "author_thumbnail": "https://yt3.ggpht.com/example-avatar.jpg",
  "reply_count": 2
}
```

Save as `sfos/harbour-sailpipe/tests/testdata/ytdlp_comment.json`.

- [ ] **Step 2: Write `ytdlptranslate.h`**

```cpp
#ifndef YTDLPTRANSLATE_H
#define YTDLPTRANSLATE_H

#include <QJsonObject>
#include <QJsonArray>

namespace YtDlpTranslate {

QJsonObject streamItem(QJsonObject const& entry);
QJsonObject searchResults(QJsonArray const& entries, int offset, int pageSize, int totalRequested);
QJsonObject mediaInfo(QJsonObject const& info);
QJsonObject commentItem(QJsonObject const& comment);
QJsonObject commentResults(QJsonArray const& comments, int offset, int pageSize);
QJsonObject channelInfo(QJsonObject const& info);
QJsonObject playlistExtra(QJsonObject const& info);

}

#endif // YTDLPTRANSLATE_H
```

- [ ] **Step 3: Write `ytdlptranslate.cpp`**

```cpp
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDate>

#include "ytdlptranslate.h"

namespace {

qint64 epochFromEntry(QJsonObject const& entry)
{
  if (entry.contains(QStringLiteral("timestamp")) && entry[QStringLiteral("timestamp")].isDouble()) {
    return static_cast<qint64>(entry[QStringLiteral("timestamp")].toDouble());
  }
  QString uploadDate = entry[QStringLiteral("upload_date")].toString();
  if (uploadDate.size() == 8) {
    QDate date = QDate::fromString(uploadDate, QStringLiteral("yyyyMMdd"));
    if (date.isValid()) {
      return QDateTime(date, QTime(0, 0), Qt::UTC).toSecsSinceEpoch();
    }
  }
  return 0;
}

QString urlFromEntry(QJsonObject const& entry)
{
  QString url = entry[QStringLiteral("webpage_url")].toString();
  if (url.isEmpty()) {
    QString id = entry[QStringLiteral("id")].toString();
    if (!id.isEmpty()) {
      url = QString("https://www.youtube.com/watch?v=%1").arg(id);
    }
  }
  return url;
}

QString firstNonEmpty(QString const& first, QString const& second)
{
  return first.isEmpty() ? second : first;
}

} // namespace

namespace YtDlpTranslate {

QJsonObject streamItem(QJsonObject const& entry)
{
  QJsonObject item;
  qint64 epoch = epochFromEntry(entry);

  item[QStringLiteral("infoType")] = QStringLiteral("STREAM");
  item[QStringLiteral("name")] = entry[QStringLiteral("title")].toString();
  item[QStringLiteral("thumbnails")] = entry[QStringLiteral("thumbnails")].toArray();
  item[QStringLiteral("url")] = urlFromEntry(entry);
  item[QStringLiteral("uploaderName")] = firstNonEmpty(
    entry[QStringLiteral("uploader")].toString(),
    entry[QStringLiteral("channel")].toString());

  QJsonObject uploadDate;
  uploadDate[QStringLiteral("offsetDateTime")] = epoch;
  item[QStringLiteral("uploadDate")] = uploadDate;

  item[QStringLiteral("textualUploadDate")] = epoch > 0
    ? QDateTime::fromSecsSinceEpoch(epoch, Qt::UTC).date().toString(Qt::ISODate)
    : QString();
  item[QStringLiteral("duration")] = static_cast<qint64>(entry[QStringLiteral("duration")].toDouble(0));

  return item;
}

QJsonObject searchResults(QJsonArray const& entries, int offset, int pageSize, int totalRequested)
{
  QJsonObject result;
  QJsonArray items;

  int end = qMin(entries.size(), offset + pageSize);
  for (int i = offset; i < end; ++i) {
    items.append(streamItem(entries[i].toObject()));
  }

  result[QStringLiteral("relatedItems")] = items;
  result[QStringLiteral("itemsList")] = items;

  QJsonObject nextPage;
  bool more = (end < entries.size()) || (entries.size() >= totalRequested);
  if (more) {
    nextPage[QStringLiteral("id")] = QString::number(end);
  }
  result[QStringLiteral("nextPage")] = nextPage;

  return result;
}

QJsonObject mediaInfo(QJsonObject const& info)
{
  QJsonObject result;
  qint64 epoch = epochFromEntry(info);
  QJsonArray categories = info[QStringLiteral("categories")].toArray();

  result[QStringLiteral("name")] = info[QStringLiteral("title")].toString();
  result[QStringLiteral("uploaderName")] = firstNonEmpty(
    info[QStringLiteral("uploader")].toString(),
    info[QStringLiteral("channel")].toString());
  result[QStringLiteral("category")] = categories.isEmpty() ? QString() : categories[0].toString();
  result[QStringLiteral("viewCount")] = info[QStringLiteral("view_count")].toInt(0);
  result[QStringLiteral("likeCount")] = info[QStringLiteral("like_count")].toInt(0);
  result[QStringLiteral("content")] = info[QStringLiteral("url")].toString();
  result[QStringLiteral("uploadDate")] = epoch;

  QJsonObject description;
  description[QStringLiteral("content")] = info[QStringLiteral("description")].toString();
  description[QStringLiteral("type")] = 3; // MediaInfo::PlainText
  result[QStringLiteral("description")] = description;

  result[QStringLiteral("length")] = static_cast<qint64>(info[QStringLiteral("duration")].toDouble(0));
  result[QStringLiteral("licence")] = info[QStringLiteral("license")].toString();

  return result;
}

QJsonObject commentItem(QJsonObject const& comment)
{
  QJsonObject item;

  QJsonObject commentText;
  commentText[QStringLiteral("content")] = comment[QStringLiteral("text")].toString();
  item[QStringLiteral("commentText")] = commentText;

  item[QStringLiteral("uploaderName")] = comment[QStringLiteral("author")].toString();

  QJsonArray avatars;
  QString avatarUrl = comment[QStringLiteral("author_thumbnail")].toString();
  if (!avatarUrl.isEmpty()) {
    QJsonObject avatar;
    avatar[QStringLiteral("url")] = avatarUrl;
    avatars.append(avatar);
  }
  item[QStringLiteral("uploaderAvatars")] = avatars;

  item[QStringLiteral("replyCount")] = comment[QStringLiteral("reply_count")].toInt(0);
  item[QStringLiteral("replies")] = QJsonObject();

  return item;
}

QJsonObject commentResults(QJsonArray const& comments, int offset, int pageSize)
{
  QJsonObject result;
  QJsonArray items;

  int end = qMin(comments.size(), offset + pageSize);
  for (int i = offset; i < end; ++i) {
    items.append(commentItem(comments[i].toObject()));
  }

  result[QStringLiteral("relatedItems")] = items;
  result[QStringLiteral("itemsList")] = items;

  QJsonObject nextPage;
  if (end < comments.size()) {
    nextPage[QStringLiteral("id")] = QString::number(end);
  }
  result[QStringLiteral("nextPage")] = nextPage;

  return result;
}

QJsonObject channelInfo(QJsonObject const& info)
{
  QJsonObject result;

  result[QStringLiteral("id")] = info[QStringLiteral("channel_id")].toString();
  result[QStringLiteral("name")] = firstNonEmpty(
    info[QStringLiteral("channel")].toString(),
    info[QStringLiteral("uploader")].toString());
  result[QStringLiteral("url")] = info[QStringLiteral("channel_url")].toString();
  result[QStringLiteral("description")] = info[QStringLiteral("description")].toString();
  result[QStringLiteral("subscriberCount")] = info[QStringLiteral("channel_follower_count")].toInt(0);
  result[QStringLiteral("verified")] = false;
  result[QStringLiteral("tags")] = QJsonArray();
  result[QStringLiteral("tabs")] = QJsonArray();

  return result;
}

QJsonObject playlistExtra(QJsonObject const& info)
{
  QJsonObject result;

  QJsonObject description;
  description[QStringLiteral("content")] = info[QStringLiteral("description")].toString();
  description[QStringLiteral("type")] = 3; // PlaylistModel::PlainText
  result[QStringLiteral("description")] = description;

  int streamCount = info[QStringLiteral("playlist_count")].toInt(-1);
  if (streamCount < 0) {
    streamCount = info[QStringLiteral("entries")].toArray().size();
  }
  result[QStringLiteral("streamCount")] = streamCount;

  result[QStringLiteral("uploaderName")] = firstNonEmpty(
    info[QStringLiteral("uploader")].toString(),
    info[QStringLiteral("channel")].toString());
  result[QStringLiteral("uploaderAvatars")] = QJsonArray();

  return result;
}

} // namespace YtDlpTranslate
```

- [ ] **Step 4: Write the failing test — `tests/testytdlptranslate.cpp`**

```cpp
#include <QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>

#include "../src/ytdlptranslate.h"

class TestYtDlpTranslate : public QObject
{
  Q_OBJECT

private:
  QJsonObject loadFixture(QString const& name)
  {
    QFile file(QString("%1/testdata/%2").arg(TESTDATA_DIR, name));
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object();
  }

private slots:
  void streamItem_mapsTitleAndUrl();
  void streamItem_fallsBackToWatchUrlWhenWebpageUrlMissing();
  void searchResults_setsNextPageWhenMoreAvailable();
  void searchResults_omitsNextPageWhenExhausted();
  void mediaInfo_mapsContentFromResolvedUrl();
  void commentItem_mapsAuthorAndText();
};

void TestYtDlpTranslate::streamItem_mapsTitleAndUrl()
{
  QJsonObject entry = loadFixture("ytdlp_search_entry.json");

  QJsonObject result = YtDlpTranslate::streamItem(entry);

  QCOMPARE(result["infoType"].toString(), QStringLiteral("STREAM"));
  QVERIFY(!result["name"].toString().isEmpty());
  QVERIFY(result["url"].toString().startsWith(QStringLiteral("https://www.youtube.com/watch?v=")));
}

void TestYtDlpTranslate::streamItem_fallsBackToWatchUrlWhenWebpageUrlMissing()
{
  QJsonObject entry;
  entry["id"] = "abc123";
  entry["title"] = "Test video";

  QJsonObject result = YtDlpTranslate::streamItem(entry);

  QCOMPARE(result["url"].toString(), QStringLiteral("https://www.youtube.com/watch?v=abc123"));
}

void TestYtDlpTranslate::searchResults_setsNextPageWhenMoreAvailable()
{
  QJsonArray entries;
  for (int i = 0; i < 5; ++i) {
    QJsonObject entry;
    entry["id"] = QString("video%1").arg(i);
    entry["title"] = QString("Video %1").arg(i);
    entries.append(entry);
  }

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, 3, 5);

  QCOMPARE(result["relatedItems"].toArray().size(), 3);
  QVERIFY(result["nextPage"].toObject().contains("id"));
  QCOMPARE(result["nextPage"].toObject()["id"].toString(), QStringLiteral("3"));
}

void TestYtDlpTranslate::searchResults_omitsNextPageWhenExhausted()
{
  QJsonArray entries;
  QJsonObject entry;
  entry["id"] = "onlyvideo";
  entry["title"] = "Only Video";
  entries.append(entry);

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, 10, 10);

  QCOMPARE(result["relatedItems"].toArray().size(), 1);
  QVERIFY(!result["nextPage"].toObject().contains("id"));
}

void TestYtDlpTranslate::mediaInfo_mapsContentFromResolvedUrl()
{
  QJsonObject info = loadFixture("ytdlp_video_info.json");

  QJsonObject result = YtDlpTranslate::mediaInfo(info);

  QVERIFY(!result["name"].toString().isEmpty());
  QVERIFY(!result["content"].toString().isEmpty());
  QCOMPARE(result["description"].toObject()["type"].toInt(), 3);
}

void TestYtDlpTranslate::commentItem_mapsAuthorAndText()
{
  QJsonObject comment = loadFixture("ytdlp_comment.json");

  QJsonObject result = YtDlpTranslate::commentItem(comment);

  QCOMPARE(result["uploaderName"].toString(), QStringLiteral("Someone"));
  QCOMPARE(result["commentText"].toObject()["content"].toString(),
    QStringLiteral("Great video, thanks for sharing!"));
  QCOMPARE(result["replyCount"].toInt(), 2);
}

QTEST_APPLESS_MAIN(TestYtDlpTranslate)
#include "testytdlptranslate.moc"
```

- [ ] **Step 5: Write `tests/CMakeLists.txt`**

```cmake
find_package(Qt5 COMPONENTS Core Test REQUIRED)

set(CMAKE_AUTOMOC ON)

add_executable(test_ytdlptranslate
    testytdlptranslate.cpp
    "${CMAKE_SOURCE_DIR}/src/ytdlptranslate.cpp"
)
target_compile_definitions(test_ytdlptranslate PRIVATE
    TESTDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(test_ytdlptranslate
    Qt5::Core
    Qt5::Test
)
add_test(NAME ytdlptranslate COMMAND test_ytdlptranslate)
```

- [ ] **Step 6: Wire the test subdirectory into the main `CMakeLists.txt`**

Modify `sfos/harbour-sailpipe/CMakeLists.txt` — add near the top, after `project(harbour-sailpipe CXX)` at line 2:

```cmake
enable_testing()
```

And near the end, after the `install(...)` blocks (after line 120), before the `distfiles` custom target:

```cmake
add_subdirectory(tests)
```

- [ ] **Step 7: MANUAL VERIFICATION — run the test**

Not runnable in this session (no Qt5 dev packages here). In your Sailfish SDK build environment (this test only needs Qt5Core/Qt5Test, so it may also build in a plain desktop Qt5 environment if you have one — doesn't require the Sailfish/Silica libraries):

```bash
cd sfos/harbour-sailpipe
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target test_ytdlptranslate
ctest -R ytdlptranslate --output-on-failure
```

Expected: all 6 tests pass. If `mediaInfo_mapsContentFromResolvedUrl` or the `streamItem_mapsTitleAndUrl` fixture-based tests fail, check the fixture JSON in `tests/testdata/` actually has the expected fields (yt-dlp's output shape can vary by version) and adjust either the fixture capture command in Step 1 or the translate function to match reality — the fixture is ground truth, not the code.

- [ ] **Step 8: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlptranslate.h sfos/harbour-sailpipe/src/ytdlptranslate.cpp \
        sfos/harbour-sailpipe/tests/ sfos/harbour-sailpipe/CMakeLists.txt
git commit -m "Add YtDlpTranslate JSON mapping functions with tests"
```

---

### Task 4: `YtDlpProcess` — subprocess runner

**Files:**
- Create: `sfos/harbour-sailpipe/src/ytdlpprocess.h`
- Create: `sfos/harbour-sailpipe/src/ytdlpprocess.cpp`

**Interfaces:**
- Consumes: `YtDlpManager::binaryPath()` (Task 1).
- Produces: `YtDlpProcess::run(QStringList const& args) -> YtDlpProcess::Result` where `Result` has `bool success`, `QJsonDocument output`, `QString errorMessage`. Consumed by `YtDlpBackend` (Tasks 5-9).

- [ ] **Step 1: Write `ytdlpprocess.h`**

```cpp
#ifndef YTDLPPROCESS_H
#define YTDLPPROCESS_H

#include <QString>
#include <QStringList>
#include <QJsonDocument>

class YtDlpProcess
{
public:
  struct Result {
    bool success;
    QJsonDocument output;
    QString errorMessage;
  };

  static Result run(QStringList const& args, int timeoutMs = 30000);
};

#endif // YTDLPPROCESS_H
```

- [ ] **Step 2: Write `ytdlpprocess.cpp`**

```cpp
#include <QProcess>
#include <QFileInfo>

#include "ytdlpmanager.h"
#include "ytdlpprocess.h"

YtDlpProcess::Result YtDlpProcess::run(QStringList const& args, int timeoutMs)
{
  Result result{false, QJsonDocument(), QString()};

  QString binary = YtDlpManager::binaryPath();
  if (!QFileInfo(binary).isExecutable()) {
    result.errorMessage = QStringLiteral("yt-dlp is not installed");
    return result;
  }

  QProcess process;
  process.start(binary, args);

  if (!process.waitForFinished(timeoutMs)) {
    process.kill();
    result.errorMessage = QStringLiteral("yt-dlp timed out");
    return result;
  }

  if (process.exitCode() != 0) {
    QString stderrOutput = QString::fromUtf8(process.readAllStandardError());
    QStringList lines = stderrOutput.split(QChar('\n'), Qt::SkipEmptyParts);
    result.errorMessage = lines.isEmpty() ? QStringLiteral("yt-dlp failed") : lines.last();
    return result;
  }

  QByteArray stdOut = process.readAllStandardOutput();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    result.errorMessage = QString("Failed to parse yt-dlp output: %1").arg(parseError.errorString());
    return result;
  }

  result.success = true;
  result.output = doc;
  return result;
}
```

- [ ] **Step 3: MANUAL VERIFICATION**

No automated test — this is a thin subprocess wrapper whose behaviour depends entirely on the real `yt-dlp` binary and OS process semantics; per project conventions we don't mock system boundaries like this internally. Verify manually once Task 12 wires it into the build: install yt-dlp via the Settings page, then confirm `YtDlpProcess::run({"--version"})` (temporarily call from a debug button, or from `YtDlpBackend` in Task 5) returns `success == true`.

- [ ] **Step 4: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpprocess.h sfos/harbour-sailpipe/src/ytdlpprocess.cpp
git commit -m "Add YtDlpProcess subprocess runner"
```

---

### Task 5: `YtDlpBackend` — search operations

**Files:**
- Create: `sfos/harbour-sailpipe/src/ytdlpbackend.h`
- Create: `sfos/harbour-sailpipe/src/ytdlpbackend.cpp`

**Interfaces:**
- Consumes: `YtDlpProcess::run` (Task 4), `YtDlpTranslate::searchResults` (Task 3).
- Produces: `YtDlpBackend::invoke(QString const& methodName, QJsonDocument const& in) -> QJsonDocument`, matching the exact signature `Invoke::run()` produces via `Extractor::invokeSync`/`invokeAsync` (see `extractor.cpp:101-111`). Consumed by `Extractor` (Task 10).

This task implements `searchFor` and `getMoreSearchItems`; Tasks 6-9 add the other operation branches to the same `invoke()` dispatch.

- [ ] **Step 1: Write `ytdlpbackend.h`**

```cpp
#ifndef YTDLPBACKEND_H
#define YTDLPBACKEND_H

#include <QJsonDocument>
#include <QString>

class YtDlpBackend
{
public:
  static QJsonDocument invoke(QString const& methodName, QJsonDocument const& in);

private:
  static QJsonDocument searchFor(QJsonObject const& in);
  static QJsonDocument getMoreSearchItems(QJsonObject const& in);
  static QJsonDocument downloadExtract(QJsonObject const& in);
  static QJsonDocument getCommentsInfo(QJsonObject const& in);
  static QJsonDocument getMoreCommentItems(QJsonObject const& in);
  static QJsonDocument getChannelInfo(QJsonObject const& in);
  static QJsonDocument getChannelTabInfo(QJsonObject const& in);
  static QJsonDocument getMoreChannelItems(QJsonObject const& in);
  static QJsonDocument getPlaylistInfo(QJsonObject const& in);
  static QJsonDocument getMorePlaylistItems(QJsonObject const& in);
  static QJsonDocument getAvailableContentFilter();

  static int pageOffset(QJsonObject const& in);
};

#endif // YTDLPBACKEND_H
```

- [ ] **Step 2: Write `ytdlpbackend.cpp`** (search operations only — other methods stubbed to return an empty envelope so the file compiles standalone; Tasks 6-9 fill them in)

```cpp
#include <QJsonObject>
#include <QJsonArray>

#include "ytdlpprocess.h"
#include "ytdlptranslate.h"
#include "ytdlpbackend.h"

#define SEARCH_PAGE_SIZE 20

int YtDlpBackend::pageOffset(QJsonObject const& in)
{
  QJsonObject page = in[QStringLiteral("page")].toObject();
  bool ok = false;
  int offset = page[QStringLiteral("id")].toString().toInt(&ok);
  return ok ? offset : 0;
}

QJsonDocument YtDlpBackend::invoke(QString const& methodName, QJsonDocument const& in)
{
  QJsonObject inObject = in.object();
  QJsonObject result;

  if (methodName == QStringLiteral("searchFor")) {
    return searchFor(inObject);
  }
  else if (methodName == QStringLiteral("getMoreSearchItems")) {
    return getMoreSearchItems(inObject);
  }
  else if (methodName == QStringLiteral("downloadExtract")) {
    return downloadExtract(inObject);
  }
  else if (methodName == QStringLiteral("getCommentsInfo")) {
    return getCommentsInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMoreCommentItems")) {
    return getMoreCommentItems(inObject);
  }
  else if (methodName == QStringLiteral("getChannelInfo")) {
    return getChannelInfo(inObject);
  }
  else if (methodName == QStringLiteral("getChannelTabInfo")) {
    return getChannelTabInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMoreChannelItems")) {
    return getMoreChannelItems(inObject);
  }
  else if (methodName == QStringLiteral("getPlaylistInfo")) {
    return getPlaylistInfo(inObject);
  }
  else if (methodName == QStringLiteral("getMorePlaylistItems")) {
    return getMorePlaylistItems(inObject);
  }
  else if (methodName == QStringLiteral("getAvailableContentFilter")) {
    return getAvailableContentFilter();
  }
  else if (methodName == QStringLiteral("tearDown")) {
    return QJsonDocument(QJsonObject());
  }

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::searchFor(QJsonObject const& in)
{
  QString searchTerm = in[QStringLiteral("searchString")].toString();
  int requested = SEARCH_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << QString("ytsearch%1:%2").arg(requested).arg(searchTerm));

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, SEARCH_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreSearchItems(QJsonObject const& in)
{
  QString searchTerm = in[QStringLiteral("searchString")].toString();
  int offset = pageOffset(in);
  int requested = offset + SEARCH_PAGE_SIZE;

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << QString("ytsearch%1:%2").arg(requested).arg(searchTerm));

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, SEARCH_PAGE_SIZE, requested);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::downloadExtract(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getCommentsInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMoreCommentItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getChannelInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getChannelTabInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMoreChannelItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getPlaylistInfo(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getMorePlaylistItems(QJsonObject const& in)
{
  Q_UNUSED(in)
  return QJsonDocument(QJsonObject());
}

QJsonDocument YtDlpBackend::getAvailableContentFilter()
{
  return QJsonDocument(QJsonObject());
}
```

Note: `ytsearchN:` re-fetches from scratch each time N grows, since yt-dlp's search has no continuation token (documented gap in the design doc). This means "load more" on a long search re-does the whole search with a bigger N — acceptable for the page sizes involved (20, 40, 60...) but would get wasteful for very deep pagination.

- [ ] **Step 3: MANUAL VERIFICATION**

Requires Task 12 (build wiring) and Task 10 (Extractor wiring) to actually exercise from the UI. Deferred to Task 10's verification step.

- [ ] **Step 4: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpbackend.h sfos/harbour-sailpipe/src/ytdlpbackend.cpp
git commit -m "Add YtDlpBackend with search operations"
```

---

### Task 6: `YtDlpBackend` — video info (`downloadExtract`)

**Files:**
- Modify: `sfos/harbour-sailpipe/src/ytdlpbackend.cpp` (replace the `downloadExtract` stub from Task 5)

**Interfaces:**
- Consumes: `YtDlpTranslate::mediaInfo` (Task 3).

- [ ] **Step 1: Replace the `downloadExtract` method**

```cpp
QJsonDocument YtDlpBackend::downloadExtract(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("-f")
    << QStringLiteral("best[ext=mp4]/best")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonObject result = YtDlpTranslate::mediaInfo(process.output.object());
  return QJsonDocument(result);
}
```

`-f "best[ext=mp4]/best"` requests a single progressive (pre-muxed audio+video) format so `content` is one directly playable URL for the existing `VideoPlayer` QML element — no manifest/DASH handling needed on the Qt side, matching what NewPipeExtractor already provided.

- [ ] **Step 2: MANUAL VERIFICATION**

Deferred to Task 10.

- [ ] **Step 3: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpbackend.cpp
git commit -m "Implement YtDlpBackend downloadExtract (video info + playback URL)"
```

---

### Task 7: `YtDlpBackend` — comments

**Files:**
- Modify: `sfos/harbour-sailpipe/src/ytdlpbackend.cpp` (replace `getCommentsInfo`/`getMoreCommentItems` stubs)

**Interfaces:**
- Consumes: `YtDlpTranslate::commentResults` (Task 3).

- [ ] **Step 1: Replace the comment methods**

```cpp
#define COMMENT_PAGE_SIZE 20

QJsonDocument YtDlpBackend::getCommentsInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--write-comments")
    << QStringLiteral("--extractor-args")
    << QStringLiteral("youtube:max_comments=100")
    << QStringLiteral("-J")
    << QStringLiteral("--no-download")
    << url, 60000);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray comments = process.output.object()[QStringLiteral("comments")].toArray();
  QJsonObject result = YtDlpTranslate::commentResults(comments, 0, COMMENT_PAGE_SIZE);
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreCommentItems(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();
  int offset = pageOffset(in);

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--write-comments")
    << QStringLiteral("--extractor-args")
    << QStringLiteral("youtube:max_comments=100")
    << QStringLiteral("-J")
    << QStringLiteral("--no-download")
    << url, 60000);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray comments = process.output.object()[QStringLiteral("comments")].toArray();
  QJsonObject result = YtDlpTranslate::commentResults(comments, offset, COMMENT_PAGE_SIZE);
  return QJsonDocument(result);
}
```

`Extractor::appendMoreComments` (`extractor.cpp:289-319`) also calls `getMoreCommentItems` — no separate backend method needed, matching the existing GraalVM path which reuses the same method name for both.

Note: this re-fetches and re-extracts *all* comments on every "load more" (yt-dlp has no incremental comment pagination token either), then slices client-side by offset. This is markedly less efficient than NewPipeExtractor's token-based paging for videos with thousands of comments — acceptable for now since `max_comments=100` bounds the cost, but worth watching if it's slow in practice.

- [ ] **Step 2: MANUAL VERIFICATION**

Deferred to Task 10.

- [ ] **Step 3: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpbackend.cpp
git commit -m "Implement YtDlpBackend comment operations"
```

---

### Task 8: `YtDlpBackend` — channel info and channel tabs

**Files:**
- Modify: `sfos/harbour-sailpipe/src/ytdlpbackend.cpp` (replace `getChannelInfo`/`getChannelTabInfo`/`getMoreChannelItems` stubs)

**Interfaces:**
- Consumes: `YtDlpTranslate::channelInfo`, `::searchResults` (Task 3).

`ChannelInfo::parseJson` (`channelinfo.cpp:126-140`) expects a `tabs` array of tab descriptors, and `ChannelTabInfo::parseJson` (`channeltabinfo.cpp:77-95`) expects `relatedItems`/`nextPage`/`contentFilters`/`sortFilter` for a specific tab. yt-dlp has no concept of channel tabs (Videos/Shorts/Live) as separate extractable entities the way NewPipeExtractor does — it just lists the channel's uploads. Scope this down: emit exactly one synthetic tab ("Videos"), so `ChannelPage.qml` still renders a working single-tab channel view rather than trying to replicate NewPipeExtractor's multi-tab structure.

- [ ] **Step 1: Replace the channel methods**

```cpp
#define CHANNEL_PAGE_SIZE 30

QJsonDocument YtDlpBackend::getChannelInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("--playlist-items")
    << QStringLiteral("0")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonObject info = process.output.object();
  QJsonObject result = YtDlpTranslate::channelInfo(info);

  QJsonObject videosTab;
  videosTab[QStringLiteral("originalUrl")] = url;
  videosTab[QStringLiteral("url")] = url;
  videosTab[QStringLiteral("id")] = QStringLiteral("videos");
  videosTab[QStringLiteral("contentFilters")] = QJsonArray{QStringLiteral("videos")};
  videosTab[QStringLiteral("sortFilter")] = QString();

  QJsonArray tabs;
  tabs.append(videosTab);
  result[QStringLiteral("tabs")] = tabs;

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getChannelTabInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, CHANNEL_PAGE_SIZE, entries.size());
  result[QStringLiteral("contentFilters")] = QJsonArray{QStringLiteral("videos")};
  result[QStringLiteral("sortFilter")] = QString();

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMoreChannelItems(QJsonObject const& in)
{
  QJsonObject linkHandler = in;
  QString url = linkHandler[QStringLiteral("url")].toString();
  int offset = pageOffset(in);

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, CHANNEL_PAGE_SIZE, entries.size());
  return QJsonDocument(result);
}
```

`--playlist-items 0` in `getChannelInfo` fetches channel metadata without pulling the full video list (cheaper — the video list is fetched separately by `getChannelTabInfo` when the user opens the tab). Both `getChannelTabInfo` and `getMoreChannelItems` re-fetch the whole flat channel listing per call (same "no incremental token" tradeoff as search/comments) and slice client-side.

- [ ] **Step 2: MANUAL VERIFICATION**

Deferred to Task 10.

- [ ] **Step 3: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpbackend.cpp
git commit -m "Implement YtDlpBackend channel operations"
```

---

### Task 9: `YtDlpBackend` — playlists and content filters

**Files:**
- Modify: `sfos/harbour-sailpipe/src/ytdlpbackend.cpp` (replace `getPlaylistInfo`/`getMorePlaylistItems`/`getAvailableContentFilter` stubs)

**Interfaces:**
- Consumes: `YtDlpTranslate::searchResults`, `::playlistExtra` (Task 3).

`Extractor::getPlaylistInfo` (`extractor.cpp:426-462`) reads `relatedItems`/`nextPage` from the result for the item list *and* separately calls `playlistModel->parseJson(result.object())` for the description/streamCount/uploader fields — both come from the same JSON object, so `getPlaylistInfo` here returns a merged object with both sets of keys.

- [ ] **Step 1: Replace the playlist and filter methods**

```cpp
#define PLAYLIST_PAGE_SIZE 30

QJsonDocument YtDlpBackend::getPlaylistInfo(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonObject info = process.output.object();
  QJsonArray entries = info[QStringLiteral("entries")].toArray();

  QJsonObject result = YtDlpTranslate::searchResults(entries, 0, PLAYLIST_PAGE_SIZE, entries.size());
  QJsonObject extra = YtDlpTranslate::playlistExtra(info);
  for (auto it = extra.constBegin(); it != extra.constEnd(); ++it) {
    result[it.key()] = it.value();
  }

  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getMorePlaylistItems(QJsonObject const& in)
{
  QString url = in[QStringLiteral("url")].toString();
  int offset = pageOffset(in);

  YtDlpProcess::Result process = YtDlpProcess::run(QStringList()
    << QStringLiteral("--flat-playlist")
    << QStringLiteral("-J")
    << url);

  if (!process.success) {
    return QJsonDocument(QJsonObject());
  }

  QJsonArray entries = process.output.object()[QStringLiteral("entries")].toArray();
  QJsonObject result = YtDlpTranslate::searchResults(entries, offset, PLAYLIST_PAGE_SIZE, entries.size());
  return QJsonDocument(result);
}

QJsonDocument YtDlpBackend::getAvailableContentFilter()
{
  QJsonObject result;
  result[QStringLiteral("stringList")] = QJsonArray{QStringLiteral("all")};
  return QJsonDocument(result);
}
```

`getAvailableContentFilter` returns a fixed single "all" filter — yt-dlp's `ytsearch` has no equivalent to NewPipeExtractor's per-service filter discovery API, and YouTube search doesn't expose meaningful server-side content-type filters through yt-dlp's search interface. This matches the design doc's stated scope for this operation.

- [ ] **Step 2: MANUAL VERIFICATION**

Deferred to Task 10.

- [ ] **Step 3: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpbackend.cpp
git commit -m "Implement YtDlpBackend playlist operations and content filter"
```

---

### Task 10: Wire `YtDlpBackend` into `Extractor`

**Files:**
- Modify: `sfos/harbour-sailpipe/src/extractor.h:67-71` (change `invokeSync`/`invokeAsync` signatures — no change needed, they already return the right type; add branch logic in `.cpp`)
- Modify: `sfos/harbour-sailpipe/src/extractor.cpp:101-111` (`invokeSync`/`invokeAsync`)

**Interfaces:**
- Consumes: `YtDlpBackend::invoke` (Task 5).

- [ ] **Step 1: Add the include**

At the top of `sfos/harbour-sailpipe/src/extractor.cpp`, alongside the existing includes (after line 20, before `#include "extractor.h"`):

```cpp
#include "ytdlpbackend.h"
```

- [ ] **Step 2: Branch `invokeSync`/`invokeAsync` on service**

Replace `extractor.cpp:101-111`:

```cpp
QJsonDocument Extractor::invokeSync(QString const methodName, QJsonDocument const* in)
{
  if (m_service == YouTubeService) {
    return YtDlpBackend::invoke(methodName, *in);
  }

  Invoke* invoke = new Invoke(this, methodName, in);
  return invoke->run();
}

QFuture<QJsonDocument> Extractor::invokeAsync(QString const methodName, QJsonDocument const* in)
{
  if (m_service == YouTubeService) {
    QJsonDocument document = in ? *in : QJsonDocument();
    return QtConcurrent::run(&m_threadPool, [methodName, document]() {
      return YtDlpBackend::invoke(methodName, document);
    });
  }

  Invoke* invoke = new Invoke(this, methodName, in);
  return QtConcurrent::run(&m_threadPool, invoke, &Invoke::run);
}
```

Note the capture-by-value of `document` in the lambda: `in` is a caller-owned pointer whose lifetime isn't guaranteed to outlive the async call (the existing `Invoke` path sidesteps this by copying `*in` into `Invoke`'s constructor — see `invoke.cpp`), so the lambda must copy the `QJsonDocument` itself rather than capturing the pointer.

- [ ] **Step 2: MANUAL VERIFICATION — full end-to-end check**

This is the first point where the whole YouTube pipeline is wired together. Requires Task 12 (CMakeLists/registration) to be done first. In your Sailfish SDK/emulator:

1. Install yt-dlp via the new Settings page.
2. Switch service to YouTube (`ServicePage.qml`).
3. Search for a term — confirm results appear (exercises `searchFor`, Task 5).
4. Scroll to trigger "load more" — confirm more results append (exercises `getMoreSearchItems`, Task 5).
5. Open a video — confirm it plays and metadata (title, uploader, description) shows (exercises `downloadExtract`, Task 6).
6. Scroll the video page to load comments (exercises `getCommentsInfo`, Task 7).
7. Open the uploader's channel — confirm channel info and video list load (exercises `getChannelInfo`/`getChannelTabInfo`, Task 8).
8. Open a playlist — confirm playlist info and items load (exercises `getPlaylistInfo`, Task 9).

Expected: each step behaves the same as it did under NewPipeExtractor before this change, from the user's perspective. If a step fails, capture the actual `yt-dlp` command's raw JSON output (run the equivalent `yt-dlp ...` command by hand from a terminal) and compare against what `YtDlpTranslate` expects — the yt-dlp JSON schema does shift between versions, and the translate functions may need adjusting to match the installed version's actual output.

- [ ] **Step 3: Commit**

```bash
git add sfos/harbour-sailpipe/src/extractor.cpp
git commit -m "Route YouTube extraction through YtDlpBackend"
```

---

### Task 11: yt-dlp-driven downloads

**Files:**
- Create: `sfos/harbour-sailpipe/src/ytdlpdownloadcontext.h`
- Create: `sfos/harbour-sailpipe/src/ytdlpdownloadcontext.cpp`
- Modify: `sfos/harbour-sailpipe/src/downloadmanager.h:48-50,71-87` (add yt-dlp branch)
- Modify: `sfos/harbour-sailpipe/src/downloadmanager.cpp:145-172` (`downloadFile`)

**Interfaces:**
- Produces: `YtDlpDownloadContext` (QObject) — `Q_PROPERTY DownloadManager::DownloadStatus downloadStatus`, `Q_PROPERTY float progress`, signals `downloadStatusChanged(DownloadManager::DownloadStatus)`, `progressChanged(float)`, slot `cancel()`. Mirrors `DownloadContext`'s external surface (same signal signatures) so `DownloadManager` can treat both uniformly.

`DownloadManager::downloadFile(url)` is called from QML with the *resolved playback URL* (`root.source`, i.e. `mediaInfo.content` — see `VideoPage.qml:113`), not the original page URL. But `DownloadManager::m_page` already holds the original video page URL (`DownloadManager.page = url;` in `VideoPage.qml:18`, where `url` is the page's `url` property, the actual `youtube.com/watch?v=...` link). So the yt-dlp/non-yt-dlp branch decision uses `m_page` (the origin URL), while the network path continues to use the passed-in resolved `url` parameter as it does today.

- [ ] **Step 1: Write `ytdlpdownloadcontext.h`**

```cpp
#ifndef YTDLPDOWNLOADCONTEXT_H
#define YTDLPDOWNLOADCONTEXT_H

#include <QObject>
#include <QProcess>

#include "downloadmanager.h"

class YtDlpDownloadContext : public QObject
{
  Q_OBJECT

  Q_PROPERTY(DownloadManager::DownloadStatus downloadStatus READ downloadStatus NOTIFY downloadStatusChanged)

public:
  explicit YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, QObject *parent = nullptr);
  ~YtDlpDownloadContext();

  void start();
  void cancel();

  QString page() const;
  DownloadManager::DownloadStatus downloadStatus() const;
  float progress() const;

signals:
  void downloadStatusChanged(DownloadManager::DownloadStatus downloadStatus);
  void progressChanged(float progress);
  void finalise();

private slots:
  void onReadyReadStandardOutput();
  void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  void setDownloadStatus(DownloadManager::DownloadStatus downloadStatus);
  void setProgress(float progress);

private:
  QString m_page;
  QString m_sourceUrl;
  QString m_targetPath;
  QProcess* m_process;
  DownloadManager::DownloadStatus m_downloadStatus;
  float m_progress;
};

#endif // YTDLPDOWNLOADCONTEXT_H
```

- [ ] **Step 2: Write `ytdlpdownloadcontext.cpp`**

```cpp
#include <QRegularExpression>

#include "ytdlpmanager.h"
#include "ytdlpdownloadcontext.h"

YtDlpDownloadContext::YtDlpDownloadContext(QString const& page, QString const& sourceUrl, QString const& targetPath, QObject *parent)
  : QObject(parent)
  , m_page(page)
  , m_sourceUrl(sourceUrl)
  , m_targetPath(targetPath)
  , m_process(new QProcess(this))
  , m_downloadStatus(DownloadManager::None)
  , m_progress(0.0)
{
  connect(m_process, &QProcess::readyReadStandardOutput, this, &YtDlpDownloadContext::onReadyReadStandardOutput);
  connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &YtDlpDownloadContext::onFinished);
}

YtDlpDownloadContext::~YtDlpDownloadContext()
{
}

void YtDlpDownloadContext::start()
{
  setDownloadStatus(DownloadManager::Running);

  QStringList args;
  args << QStringLiteral("-f") << QStringLiteral("best[ext=mp4]/best")
       << QStringLiteral("-o") << m_targetPath
       << QStringLiteral("--newline")
       << QStringLiteral("--progress-template") << QStringLiteral("download:PROGRESS %(progress._percent_str)s")
       << m_sourceUrl;

  m_process->start(YtDlpManager::binaryPath(), args);
}

void YtDlpDownloadContext::cancel()
{
  if (m_process->state() != QProcess::NotRunning) {
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
      m_process->kill();
    }
  }
  setDownloadStatus(DownloadManager::Cancelled);
  emit finalise();
}

QString YtDlpDownloadContext::page() const
{
  return m_page;
}

DownloadManager::DownloadStatus YtDlpDownloadContext::downloadStatus() const
{
  return m_downloadStatus;
}

float YtDlpDownloadContext::progress() const
{
  return m_progress;
}

void YtDlpDownloadContext::setDownloadStatus(DownloadManager::DownloadStatus downloadStatus)
{
  if (m_downloadStatus != downloadStatus) {
    m_downloadStatus = downloadStatus;
    emit downloadStatusChanged(m_downloadStatus);
  }
}

void YtDlpDownloadContext::setProgress(float progress)
{
  if (m_progress != progress) {
    m_progress = progress;
    emit progressChanged(m_progress);
  }
}

void YtDlpDownloadContext::onReadyReadStandardOutput()
{
  static QRegularExpression const percentPattern(QStringLiteral("PROGRESS\\s+([0-9.]+)%"));

  QByteArray data = m_process->readAllStandardOutput();
  QStringList lines = QString::fromUtf8(data).split(QChar('\n'), Qt::SkipEmptyParts);
  for (QString const& line : lines) {
    QRegularExpressionMatch match = percentPattern.match(line);
    if (match.hasMatch()) {
      setProgress(match.captured(1).toFloat() / 100.0f);
    }
  }
}

void YtDlpDownloadContext::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if ((exitStatus == QProcess::NormalExit) && (exitCode == 0)) {
    setProgress(1.0);
    setDownloadStatus(DownloadManager::Done);
  }
  else if (m_downloadStatus != DownloadManager::Cancelled) {
    setDownloadStatus(DownloadManager::Error);
  }
  emit finalise();
}
```

`--progress-template` with a fixed `PROGRESS NN.N%` prefix gives a reliably parseable line regardless of yt-dlp's default human-readable progress bar formatting, which changes with terminal width detection and isn't meant for machine parsing.

- [ ] **Step 3: Add the yt-dlp branch to `DownloadManager`**

Modify `sfos/harbour-sailpipe/src/downloadmanager.h` — add near the other private members (after `bool m_dbusRegistered;` at line 87):

```cpp
  YtDlpDownloadContext* m_ytDlpContext;
```

And add the forward declaration near the top (alongside `class DownloadContext;` at line 12):

```cpp
class YtDlpDownloadContext;
```

Modify `sfos/harbour-sailpipe/src/downloadmanager.cpp` — add the include (alongside the existing includes at the top):

```cpp
#include <QRegularExpression>
#include <QStandardPaths>

#include "ytdlpdownloadcontext.h"
```

(`QStandardPaths` is already included at line 5 — skip re-adding it if so; check before editing.)

Replace `downloadFile` (`downloadmanager.cpp:145-172`):

```cpp
void DownloadManager::downloadFile(QString const url)
{
  static QRegularExpression const youtubeHost(QStringLiteral("(^|\\.)youtube\\.com$|(^|\\.)youtu\\.be$"));

  QUrl pageUrl(m_page);
  bool isYouTube = youtubeHost.match(pageUrl.host()).hasMatch();

  if (isYouTube) {
    QString extension = QStringLiteral("mp4");
    QString targetPath = constructFilename(m_name, extension);

    m_ytDlpContext = new YtDlpDownloadContext(m_page, m_page, targetPath, this);
    connect(m_ytDlpContext, &YtDlpDownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    connect(m_ytDlpContext, &YtDlpDownloadContext::progressChanged, this, &DownloadManager::setProgress);
    connect(m_ytDlpContext, &YtDlpDownloadContext::finalise, this, &DownloadManager::onFinalise);

    setProgress(0.0);
    m_ytDlpContext->start();
    return;
  }

  QNetworkRequest request;

  qDebug() << "Download URL: " << url;
  request.setUrl(QUrl(url));
  request.setMaximumRedirectsAllowed(50);
  request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, QVariant(false));

  QNetworkReply* reply = m_manager->get(request);

  connect(reply, &QNetworkReply::readyRead, this, &DownloadManager::onReadyRead);
  connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
  connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadManager::onError);

  DownloadContext* context = new DownloadContext(m_page, reply, m_transferClient, m_dbusRegistered, this);
  reply->setProperty("context", QVariant::fromValue(context));

  if (m_running.contains(m_page)) {
    destroyContext(m_running.value(m_page));
  }
  m_running.insert(m_page, context);

  connect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
  connect(context, &DownloadContext::finalise, this, &DownloadManager::onFinalise);
  setProgress(0.0);
  setDownloadStatus(DownloadStatus::Running);
}
```

`onFinalise` (`downloadmanager.cpp:264-275`) calls `dynamic_cast<DownloadContext*>(sender())`, which returns `nullptr` for a `YtDlpDownloadContext*` sender since the two classes are unrelated — it would skip the `destroyContext`/`m_running`/`m_transferIds` cleanup (harmless, since a yt-dlp download was never added to those maps) but must still flip status back to `None` when it's the current page. Update `onFinalise` in `downloadmanager.cpp:264-275`:

```cpp
void DownloadManager::onFinalise()
{
  DownloadContext* context = dynamic_cast<DownloadContext*>(sender());
  if (context) {
    if (m_page == context->page()) {
      setDownloadStatus(None);
    }
    disconnect(context, &DownloadContext::downloadStatusChanged, this, &DownloadManager::setDownloadStatus);
    disconnect(context, &DownloadContext::finalise, this, &DownloadManager::onFinalise);
    destroyContext(context);
    return;
  }

  YtDlpDownloadContext* ytDlpContext = dynamic_cast<YtDlpDownloadContext*>(sender());
  if (ytDlpContext) {
    if (m_page == ytDlpContext->page()) {
      setDownloadStatus(None);
    }
    if (m_ytDlpContext == ytDlpContext) {
      m_ytDlpContext = nullptr;
    }
    ytDlpContext->deleteLater();
  }
}
```

And `cancel()` (`downloadmanager.cpp:290-301`) needs a yt-dlp branch too:

```cpp
void DownloadManager::cancel()
{
  qDebug() << "DownloadManager cancel: " << m_page;
  if (m_ytDlpContext && (m_ytDlpContext->page() == m_page)) {
    m_ytDlpContext->cancel();
  }
  else if (m_running.contains(m_page)) {
    DownloadContext* context = m_running.value(m_page);
    QNetworkReply* reply = context->reply();
    reply->abort();
  }
  else {
    qDebug() << "Page to cancel not found: " << m_page;
  }
}
```

Initialize `m_ytDlpContext` in the constructor — modify `DownloadManager::DownloadManager` (`downloadmanager.cpp:28-47`) member init list to add `, m_ytDlpContext(nullptr)` after `, m_dbusRegistered(false)`.

- [ ] **Step 4: MANUAL VERIFICATION**

In your Sailfish SDK/emulator: open a YouTube video, press download. Confirm the progress bar advances (driven by `YtDlpDownloadContext::progressChanged`), the file lands in the Downloads directory as a playable `.mp4`, and pressing the cancel button actually terminates the `yt-dlp` process (check `ps`/`pgrep yt-dlp` on-device mid-download, confirm it's gone after cancel). Also verify a non-YouTube download (e.g. SoundCloud) still goes through the original `QNetworkAccessManager` path unaffected.

Known limitation to confirm you're OK with: yt-dlp-driven downloads don't register with Sailfish's `nemo-transferengine`, so they won't show up in the system Transfers app the way NewPipeExtractor-era downloads did (transfer-engine's `TransferEngineClient::open()` API expects a known content-length up front and expects the caller to stream bytes through it — yt-dlp is writing the file directly, so that integration doesn't carry over cleanly). This was flagged as an acceptable simplification in the design doc's download-flow section.

- [ ] **Step 5: Commit**

```bash
git add sfos/harbour-sailpipe/src/ytdlpdownloadcontext.h sfos/harbour-sailpipe/src/ytdlpdownloadcontext.cpp \
        sfos/harbour-sailpipe/src/downloadmanager.h sfos/harbour-sailpipe/src/downloadmanager.cpp
git commit -m "Route YouTube file downloads through yt-dlp"
```

---

### Task 12: Build system wiring — CMake, QML registration, RPM spec

**Files:**
- Modify: `sfos/harbour-sailpipe/CMakeLists.txt:40-70` (source list)
- Modify: `sfos/harbour-sailpipe/src/harbour-sailpipe.cpp:20-54` (includes, `instantiate()`, `qmlRegisterSingletonType`)
- Modify: `sfos/harbour-sailpipe/rpm/harbour-sailpipe.spec` (description update only — no new BuildRequires needed, `Qt5Network` is already used unconditionally by the existing `DownloadManager` without a dedicated `BuildRequires` entry, so the same holds for `YtDlpManager`)

**Interfaces:**
- Consumes: everything from Tasks 1-11.

- [ ] **Step 1: Add new sources to `CMakeLists.txt`**

Modify `sfos/harbour-sailpipe/CMakeLists.txt:40-70` — add to the `target_sources` list (after `"src/mediajunction.cpp"` at line 69):

```cmake
    "src/mediajunction.cpp"
    "src/ytdlpmanager.cpp"
    "src/ytdlpprocess.cpp"
    "src/ytdlptranslate.cpp"
    "src/ytdlpbackend.cpp"
    "src/ytdlpdownloadcontext.cpp"
)
```

(Replacing just the closing `)` and preceding line — the five new `.cpp` files are added, header files don't need listing since `CMAKE_AUTOMOC` picks them up via the `.cpp` includes.)

- [ ] **Step 2: Register `YtDlpManager` as a QML singleton**

Modify `sfos/harbour-sailpipe/src/harbour-sailpipe.cpp` — add the include after `#include "mediajunction.h"` (line 21):

```cpp
#include "ytdlpmanager.h"
```

Add instantiation after `MediaJunction::instantiate();` (line 36):

```cpp
  YtDlpManager::instantiate();
```

Add registration after the `MediaJunction` registration (line 54):

```cpp
  qmlRegisterSingletonType<YtDlpManager>("harbour.sailpipe.extractor", 1, 0, "YtDlp", YtDlpManager::provider);
```

- [ ] **Step 3: Update the RPM spec description**

Modify `sfos/harbour-sailpipe/rpm/harbour-sailpipe.spec:20-23` — the `%description` block mentions NewPipe Extractor covering "YouTube, SoundCloud, Media.ccc.de and Bandcamp"; update to reflect the split:

```
%description
Provides a Sailfish user interface for streaming and downloading video and
music from multiple online services. YouTube is handled via yt-dlp;
SoundCloud, Media.ccc.de, PeerTube and Bandcamp are handled via the NewPipe
Extractor.
```

No `BuildRequires` changes: `YtDlpManager`/`YtDlpDownloadContext` use `QNetworkAccessManager` and `QProcess`, both already implicitly available since `DownloadManager` already uses `QNetworkAccessManager` today without a dedicated `pkgconfig(Qt5Network)` entry in the spec.

- [ ] **Step 4: MANUAL VERIFICATION — full build**

```bash
# Inside your Sailfish Platform SDK / mb2 build target:
cd sfos/harbour-sailpipe
mb2 build
```

Expected: clean build, no missing-symbol or QML-registration errors. Then run through the full manual verification checklist from Task 10 Step 2 and Task 11 Step 4 on-device or in the emulator, plus:

- Package the RPM (`mb2 build` produces one, or use the project's normal packaging flow) and confirm `rpm -qlp` lists all the new source-derived binaries correctly (there's nothing new to package beyond the existing `harbour-sailpipe` binary — `ytdlpmanager.cpp` etc. compile into the same executable, no new files added to `%files`).
- Confirm the app still starts and the non-YouTube services (try SoundCloud) work exactly as before — this change must be a no-op for them.

- [ ] **Step 5: Commit**

```bash
git add sfos/harbour-sailpipe/CMakeLists.txt sfos/harbour-sailpipe/src/harbour-sailpipe.cpp sfos/harbour-sailpipe/rpm/harbour-sailpipe.spec
git commit -m "Wire yt-dlp backend into build and QML registration"
```

---

## Post-implementation

Once all 12 tasks are done and manually verified end-to-end (Task 10 Step 2 + Task 11 Step 4 + Task 12 Step 4 together cover the full user-facing surface), the feature matches the approved design doc. Follow-ups explicitly out of scope per the design doc: non-YouTube services moving to yt-dlp, armv7hl polish, bundling yt-dlp in the RPM itself.
