# Channel subscriptions + Filter menu removal

## Filter menu removal

`YtDlpBackend::getAvailableContentFilter()` always returns exactly one
option (`"all"`), so `FilterPage.qml` always shows a single,
already-selected choice — dead UI now that NewPipeExtractor's
multi-service filtering is gone.

Remove:
- `qml/pages/FilterPage.qml`
- The "Filter" `MenuItem` in `SearchPage.qml`'s `PullDownMenu`
- `filterModel`/`FilterModel` wiring in `SearchPage.qml`
- `FilterModel` class (`src/filtermodel.h/.cpp`) if nothing else uses it
- `Extractor::getAvailableContentFilter` and `YtDlpBackend::getAvailableContentFilter`

## Subscriptions

### Storage

New C++ singleton `SubscriptionManager` (same pattern as `YtDlpManager`/
`DownloadManager`: `instantiate()`/`getInstance()`/`provider()`,
registered as a QML singleton). Persists a JSON array to
`<AppDataLocation>/subscriptions.json`:

```json
[{"url": "...", "name": "...", "thumbnail": "..."}, ...]
```

API:
- `bool isSubscribed(QString url)`
- `void subscribe(QString url, QString name, QString thumbnail)`
- `void unsubscribe(QString url)`
- A `QAbstractListModel`-based accessor (`SubscriptionListModel`) for the
  management page — role data: url, name, thumbnail.

### ChannelPage: subscribe toggle

Icon button in the channel header row (next to the thumbnail/name),
bound to `SubscriptionManager.isSubscribed(root.url)`. Tap toggles
subscribe/unsubscribe, using the channel's already-loaded name/thumbnail.

### Search page: entry point

New `MenuItem` "Subscriptions" in `SearchPage.qml`'s `PullDownMenu`,
pushing a new `SubscriptionsPage.qml`.

### SubscriptionsPage (aggregate feed)

On `Component.onCompleted`, for each subscribed channel: reuse the
existing per-channel "Videos" fetch path (same call `ChannelModel`/
`ChannelTabInfo` already make for a single channel's Videos tab),
running concurrently across channels via the existing thread pool.
Merge all returned items into one list sorted by `uploadDate`
descending; items with no upload date (flat-playlist doesn't always
return one) sort to the end rather than corrupting the order.

Loading state: `ProcessIndicator` (existing component) while any
channel fetch is pending; results populate incrementally is out of
scope for v1 — show once all channels have responded, since partial
incremental merge-sort adds real complexity for a first cut. No
persistence of the aggregate feed itself — refetched every time the
page opens (per earlier confirmation).

Delegate: reuse `SearchDelegate` (same as channel Videos tab).

### Manage subscriptions (unsubscribe)

Pulley menu item "Manage subscriptions" on `SubscriptionsPage`, pushing
`ManageSubscriptionsPage.qml`: a plain `SilicaListView` over
`SubscriptionListModel`, each row a channel name/thumbnail with the
standard Sailfish swipe-to-remove ("remorse") pattern calling
`SubscriptionManager.unsubscribe(url)`. Tapping a row (not swiping)
opens that `ChannelPage`.

## Explicitly out of scope for v1

- Background/periodic refresh of the aggregate feed — fetch-on-open only.
- Incremental UI updates as each channel's fetch completes — all-or-nothing
  render once every channel has responded.
- Sync of subscriptions to a real YouTube account — this is a local-only
  list, unrelated to any YouTube account.
