# YouTube-only: remove NewPipeExtractor/GraalVM and other-service support

## Problem

SailReel currently supports 5 services (YouTube, SoundCloud, media.ccc.de,
PeerTube, Bandcamp) via two backends: `YtDlpBackend` for YouTube, and a
GraalVM-native-compiled NewPipeExtractor Java bridge (`appwrapper.so`) for
everything else. The user wants the app focused on YouTube only.

## Decision

Full rip-out, not a UI-only hide:

- Remove the `java/NewPipeExtractor` git submodule and `.gitmodules`.
- Remove the GraalVM native-compile CI job (`build-appwrapper`) and the
  `appwrapper.so`/header copy step from `CMakeLists.txt`.
- Remove the top-level `Makefile`, `Dockerfile`, `cpp/` console demo —
  it exists solely to exercise the NewPipeExtractor bridge standalone.
- Collapse `Extractor` (C++) to a thin wrapper directly over
  `YtDlpBackend` — no `Service` enum, no per-service routing.
- Remove `DownloadManager`'s generic (non-yt-dlp) HTTP download path
  (`DownloadContext`) — dead code once only yt-dlp downloads exist.
- Delete `ServicePage.qml` (the service switcher) entirely.
- Remove service-conditional branches from `SearchPage`, `ChannelPage`,
  `PlaylistPage`, `VideoPage` QML (title text switches, etc.) — collapse
  to the single YouTube case.
- Update README, `AboutPage.qml`, and the RPM `%description` to drop the
  multi-service framing and "NewPipe Extractor" section.

## Explicitly not touched

- `FilterPage.qml`/`FilterModel` — already generic; `YtDlpBackend`
  implements `getAvailableContentFilter()` itself. No NewPipeExtractor
  dependency here.
- Translation infrastructure, yt-dlp download/install flow, MPRIS/Transfer
  Engine integration — unrelated to this change.

## Risk / rollback

Straightforward git history rollback if needed; no data migration
concerns since removed services have no persisted user data tied to
them specifically.
