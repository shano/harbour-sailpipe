# yt-dlp backend for YouTube extraction

## Problem

`harbour-sailpipe` currently extracts from all 5 supported services (YouTube,
SoundCloud, MediaCCC, PeerTube, Bandcamp) through NewPipeExtractor, a Java
library compiled to a native shared object (`appwrapper.so`) via GraalVM and
called in-process from `Extractor` (`sfos/harbour-sailpipe/src/extractor.cpp`)
through a generic `invoke(thread, methodName, jsonIn) -> jsonOut` bridge.

YouTube changes its player/signature logic often enough that NewPipeExtractor
breaks regularly, and picking up upstream fixes requires a full GraalVM
native-image rebuild (see `Dockerfile`, `java/compile.sh`). yt-dlp tracks
YouTube's changes far more actively and ships pre-built, self-updating
binaries, making it a better fit for the service that breaks most.

## Scope

Replace the extraction backend for **YouTube only**. SoundCloud, MediaCCC,
PeerTube and Bandcamp keep using NewPipeExtractor/GraalVM unchanged — those
extractors are stable, and yt-dlp's support for MediaCCC in particular is
uncertain. The GraalVM build pipeline, `appwrapper.so`, and `cpp/` dev CLI
harness are untouched.

The UI gains the ability to download the yt-dlp binary and check for/install
updates, since the app is now responsible for keeping its own copy current.

## Architecture

`Extractor::m_service` already selects behaviour per-service at runtime. Every
extraction operation (`search`, `getChannelInfo`, `getPlaylistInfo`,
`getComments`, `downloadExtract`, etc.) funnels through one call shape:
`invoke(methodName, jsonIn) -> jsonOut`, where `jsonOut` is NewPipeExtractor-
shaped JSON (`relatedItems`, `nextPage`, `itemsList`, `stringList`, ...).
Every consuming Qt model (`SearchItem`, `ChannelInfo`, `PlaylistModel`,
`CommentItem`, `MediaInfo`) parses that exact shape, as do the QML pages.

A new `YtDlpBackend` class implements the same call shape —
`invoke(methodName, jsonIn) -> jsonOut` — but shells out to the `yt-dlp`
binary and **translates its output into the existing NewPipeExtractor JSON
contract**. `Extractor::invokeSync`/`invokeAsync` branch on `m_service`:
YouTube routes to `YtDlpBackend`, all other services keep using the existing
`Invoke`/GraalVM path unchanged.

Consequence: no changes needed to `SearchItem`, `ChannelInfo`,
`PlaylistModel`, `CommentItem`, `MediaInfo`, `PageRef`, or any QML page — they
keep consuming the same contract regardless of which backend produced it. All
new code is isolated to the backend + translation layer.

### Known contract gaps vs. NewPipeExtractor

- **Search pagination**: NewPipeExtractor returns a continuation token
  (`nextPage`) for incremental "load more." yt-dlp's `ytsearchN:query`
  returns N flat results per call with no token. `YtDlpBackend` fakes
  `nextPage` as an offset/count, re-issuing a larger search and discarding
  the overlap on "load more." This wastes some request volume on deep
  pagination but preserves the existing UI contract.
- **Channel/playlist listing**: yt-dlp's `--flat-playlist` typically returns
  the whole list in one call rather than paging incrementally.
  `YtDlpBackend` fetches once and slices client-side for "load more," rather
  than performing true incremental extraction.

## yt-dlp binary lifecycle

### Distribution

Confirmed via the GitHub releases API (`yt-dlp/yt-dlp`, tag `2026.07.04`):
yt-dlp ships standalone, self-contained binaries with no Python runtime
dependency:

- `yt-dlp_linux_aarch64` — raw executable. Primary target: current official
  Sailfish devices (Xperia 10 series and later) are aarch64.
- `yt-dlp_linux_armv7l.zip` — zipped executable. Covers older armv7hl
  devices (Jolla Phone/C, Xperia X). Best-effort: needs an unzip step; scope
  this as smaller/follow-up if it adds friction, not a blocker for the
  aarch64 path.
- `SHA2-256SUMS` — checksums for verifying downloaded binaries.

### Storage

App-private data directory, not a system package, not on `PATH`:
`~/.local/share/harbour-sailpipe/yt-dlp/yt-dlp`.

### `YtDlpManager` (new QObject, exposed to QML)

- Property `status`: `NotInstalled` / `Installed` / `Downloading` /
  `Updating` / `Error`
- Property `installedVersion` — from cached metadata or `yt-dlp --version`
- Slot `checkForUpdate()` — `GET
  https://api.github.com/repos/yt-dlp/yt-dlp/releases/latest`, compare tag
  against installed version
- Slot `install()` / `update()` — download the asset matching detected CPU
  architecture (`QSysInfo::currentCpuArchitecture()`), verify against
  `SHA2-256SUMS`, atomic replace, `chmod +x`
- Progress signal for download, following the same pattern as the existing
  `DownloadManager`

One code path handles both first install and subsequent updates (both go
through the GitHub API + download flow), rather than mixing that with
yt-dlp's own `-U` self-update mechanism, which can't perform the initial
install since there's nothing to invoke yet.

### UI

New `SettingsPage.qml` (no settings page exists today — only `AboutPage`),
reachable from the same pulley menu entry point as `AboutPage`. Shows install
status, installed version, "Check for Updates" button, "Install"/"Update"
button with progress bar, last-checked timestamp.

`AboutPage`'s "NewPipe Extractor" section gets a YouTube-specific note that
YouTube now uses yt-dlp, with version pulled live from `YtDlpManager`.

### Permissions

The sailjail profile (`harbour-sailpipe.profile`) already grants `Internet`;
this covers the GitHub API call and release-asset download. Writing the
binary to the app-private data directory needs no additional permission.

## Extraction mapping

All calls run via `QProcess`, off the UI thread, reusing the existing
`m_threadPool`/`QtConcurrent` pattern (`Extractor::invokeAsync`) so the
watcher/lifetime-check code in `extractor.cpp` barely changes.

| Operation | yt-dlp invocation | Translated into |
|---|---|---|
| Search | `ytsearchN:<term> --dump-json` (one JSON line per video) | `relatedItems[]` + synthetic offset-based `nextPage` |
| Video info / `downloadExtract` | `yt-dlp -J <url>` | `MediaInfo` fields (title, uploader, description, duration, viewCount, likeCount, uploadDate, thumbnail) + direct stream URL for playback |
| Comments | `yt-dlp -J --write-comments <url>`, read `.comments[]` | `relatedItems[]` shaped as `CommentItem` |
| Channel info | `yt-dlp -J --flat-playlist <channel-url>` | `ChannelInfo` fields |
| Channel tab / playlist items | `yt-dlp --flat-playlist -J <url>` → `.entries[]` | `itemsList`/`relatedItems` shaped as `SearchItem` |
| Content filters | No yt-dlp equivalent — hardcode a fixed list matching what YouTube search actually supports | `stringList` |

The per-operation JSON-translation methods on `YtDlpBackend` are the bulk of
the implementation work.

## Download flow

`downloadExtract` for YouTube still only resolves a direct stream URL for
**playback**, fed into the existing `VideoPlayer` QML element unchanged.

For **saving to disk**, `DownloadManager::downloadFile` for a YouTube URL is
rerouted to a new path that runs `yt-dlp -o <path> --newline <url>` and
parses its progress output (using `--progress-template` with a fixed format
for reliable parsing) into the existing `progress`/`downloadStatus`
properties, so `DownloadContext`, the pulley-menu UI, and notifications stay
unchanged. yt-dlp handles DASH audio/video muxing and throttling workarounds
that a raw HTTP GET on a resolved URL would not.

Non-YouTube downloads keep using `QNetworkAccessManager` via the existing
`DownloadManager` path, unchanged.

## Error handling

- If yt-dlp isn't installed and the user selects the YouTube service, surface
  a clear prompt directing them to the new Settings page rather than failing
  silently.
- If a yt-dlp subprocess exits non-zero, surface the last line of stderr as
  the error message (yt-dlp's error output is generally human-readable, e.g.
  "Video unavailable").
- No silent fallback to GraalVM/NewPipeExtractor for YouTube — service scope
  is fixed to yt-dlp per the Scope section above.

## Testing

Per project convention, no mocking of internals — and the subprocess/network
behaviour here is the actual thing under test, so it isn't a good mocking
candidate anyway. Plan:

- Unit-test the JSON-translation methods on `YtDlpBackend` in isolation,
  feeding them captured real yt-dlp JSON output as fixtures under
  `testdata/`, asserting the NewPipeExtractor-shaped output matches what the
  existing models expect.
- `YtDlpManager` (binary install/update) and subprocess invocation get
  manual/integration testing on-device — they touch filesystem, network, and
  real Sailfish permissions that aren't worth mocking.

## Out of scope / follow-ups

- Non-YouTube services moving to yt-dlp.
- armv7hl support may need a follow-up pass if the zip-extraction step turns
  out to be more friction than expected.
- Bundling a yt-dlp binary in the RPM itself (current design: UI-driven
  install only, nothing bundled at package-install time).
