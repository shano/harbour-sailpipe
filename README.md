<div align="center">

<img src="sfos/harbour-sailreel/icons/256x256/harbour-sailreel.png" width="128" height="128" alt="SailReel icon">

# SailReel

**Stream and download video & audio on Sailfish OS.**

[![Build and release](https://github.com/shano/harbour-sailreel/actions/workflows/release.yml/badge.svg)](https://github.com/shano/harbour-sailreel/actions/workflows/release.yml)
[![Latest release](https://img.shields.io/github/v/release/shano/harbour-sailreel)](https://github.com/shano/harbour-sailreel/releases/latest)
[![License: GPL v3](https://img.shields.io/github/license/shano/harbour-sailreel)](LICENSE)

</div>

---

> **This is a fork.** Upstream is [flypig/harbour-newpipe](https://codeberg.org/flypig/harbour-newpipe)
> on Codeberg (there packaged as `harbour-sailpipe`/"SailPipe"). Renamed here
> to avoid clashing with the upstream package if both are ever installed.
> Most of the code in this fork was written by AI coding agents (Claude)
> rather than by hand — review accordingly before trusting it with your
> accounts or data.

## What it is

A native Sailfish Silica app for searching, streaming, and downloading
video and audio from:

- **YouTube**
- **SoundCloud**
- **media.ccc.de**
- **PeerTube**
- **Bandcamp**

Search, browse channels and playlists, read comments, play video/audio
in-app with full media controls (MPRIS-integrated, so lock-screen and
Bluetooth controls work), and download things to your device via the
system Transfer Engine.

## yt-dlp integration

YouTube changes how it serves content often enough that a general-purpose
extractor library struggles to keep up. So YouTube is handled differently
from everything else:

- **YouTube** → [yt-dlp](https://github.com/yt-dlp/yt-dlp), a standalone
  binary the app downloads and manages itself (Settings → yt-dlp). Kept
  independent of app releases so YouTube support can be patched the
  moment upstream yt-dlp ships a fix, without waiting on a SailReel
  update.
- **Everything else** (SoundCloud, media.ccc.de, PeerTube, Bandcamp) →
  [NewPipe Extractor](https://github.com/TeamNewPipe/NewPipeExtractor),
  compiled to a native library via GraalVM and called in-process.

The yt-dlp binary is downloaded straight from GitHub Releases, verified
against its published SHA-256 checksum before install, and updated
on-demand from the Settings page — never bundled or auto-run without your
say-so.

## Installing

Grab the latest `.rpm` from [Releases](https://github.com/shano/harbour-sailreel/releases/latest)
and install it via Storeman, File Browser, or:

```
pkcon install-local --allow-untrusted harbour-sailreel-<version>.aarch64.rpm
```

Currently built for **aarch64** only (e.g. Xperia 10 II/III and other
64-bit ARM devices) — GraalVM, which compiles the NewPipe Extractor
native library, doesn't ship a 32-bit ARM (armv7hl) build.

## Building from source

CI builds via the Sailfish Platform SDK — see
[`.github/workflows/release.yml`](.github/workflows/release.yml) for the
full pipeline: GraalVM native-compiles the NewPipe Extractor bridge, then
[`mb2`](https://docs.sailfishos.org/Tools/Sailfish_SDK/) builds the RPM
against the target Sailfish OS release. Push a `vX.Y.Z` tag to trigger a
release build.

## Acknowledgements

- [flypig](https://codeberg.org/flypig) — original NewPipe-for-Sailfish
  author
- [legacychimera247](https://codeberg.org/legacychimera247) — contributor
- [TeamNewPipe](https://github.com/TeamNewPipe) — NewPipe Extractor
- [yt-dlp](https://github.com/yt-dlp/yt-dlp) contributors

This project uses [Weblate](https://weblate.org/) translation software to
support localisation.

## License

[GPLv3](LICENSE)
