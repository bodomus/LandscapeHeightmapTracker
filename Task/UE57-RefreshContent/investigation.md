# UE5-18 Investigation — Refresh Content

## Scope and baseline

- Workflow level: **Level 2 — editor integration**.
- Repository root: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Initial branch: `main`.
- Working branch: `codex/ue5-18-refresh-content`.
- Initial commit: `f21b38fdb20550ba0b7f14fec12f0d7b392d8eef`.
- `main` matched `origin/main` after `git fetch origin main` (`0 0` ahead/behind).
- Pre-existing untracked files were present under `Reviews/`, `Task/UE5-17-artifacts/`, and `images/`; they are outside this ticket and must remain untouched.
- Host project: `J:/Projects/UE_Projects/UE57Editor/UE57Editor.uproject`.
- Host project and plugin both target Unreal Engine 5.7.

## Graphify preflight

- Confirmed CLI: `graphify --help`.
- Existing graph was dated 2026-08-13 and predated the UE5-17 changes on current `main`.
- Refresh command: `graphify update .`.
- Result: 114 files extracted, 1393 nodes, 1860 edges, 105 communities.
- Focused queries:
  - editor command, toolbar, menu, and ToolMenus registration;
  - module startup/shutdown, notifications, and logging;
  - Asset Registry / Content Browser refresh concepts;
  - tests and build configuration;
  - `graphify explain "FLandscapeHeightmapTrackerModule"`.
- Architectural finding: editor commands, ToolMenus registration, command mapping, module lifecycle, and the existing static log category are owned by `FLandscapeHeightmapTrackerModule`; `FLandscapeHeightmapTrackerCommands` owns command declarations and labels.
- Limitation: Graphify did not contain any existing Asset Registry integration because the plugin has none. Its broad “refresh” query returned unrelated UI refresh symbols, so no runtime conclusion was taken from that query.

## CRG preflight

- Confirmed integration: repository CRG MCP tools.
- Initial incremental update reported no working-tree source changes but exposed a stale build SHA (`c5d278e...`) versus current HEAD (`f21b38f...`).
- Source diff between those SHAs included the UE5-17 height-range implementation, including module-adjacent UI files.
- A full rebuild was therefore required.
- Full rebuild result: 48 parsed files, 289 nodes, 2394 edges, 39 flows, 4 communities, no parse errors.
- The rebuilt graph matches branch `codex/ue5-18-refresh-content` at `f21b38f...`.
- CRG confirms:
  - `StartupModule()` maps commands and registers the ToolMenus startup callback.
  - `RegisterMenus()` extends `LevelEditor.MainMenu.Tools`, finds/creates the plugin section, and adds command entries.
  - `LandscapeHeightmapTrackerModule.cpp` imports `LevelEditor.h`, `ToolMenus.h`, the command class, and both tab widgets.
  - no CRG-linked tests currently cover `RegisterMenus()` or `RegisterCommands()`.
- CRG missed the callback relationship from `UToolMenus::RegisterStartupCallback` to `RegisterMenus()`; direct source inspection proves that delegate binding.

## Existing editor architecture

Source-verified flow:

1. `StartupModule()` initializes style and registers `FLandscapeHeightmapTrackerCommands`.
2. It creates one `FUICommandList` and maps `OpenPluginWindow`.
3. It registers a ToolMenus startup callback bound to `RegisterMenus()`.
4. `RegisterMenus()` extends `LevelEditor.MainMenu.Tools` and adds entries to the `LandscapeHeightmapTracker` section.
5. `ShutdownModule()` unregisters the ToolMenus callback/owner and unregisters commands.

There is no existing plugin toolbar extension. The existing Tools menu is therefore the preferred and least disruptive placement.

The module already defines `LogLandscapeHeightmapTracker`, so no new logging category is needed. No notification helper currently exists.

## Verified Unreal Engine 5.7 APIs

The installed engine is `C:/Program Files/Epic Games/UE_5.7`.

The actual UE 5.7 header `Runtime/AssetRegistry/Public/AssetRegistry/IAssetRegistry.h` declares:

```cpp
virtual void ScanPathsSynchronous(
    const TArray<FString>& InPaths,
    bool bForceRescan = false,
    bool bIgnoreDenyListScanFilters = false) = 0;
```

Its documented behavior is recursive scanning of supplied paths, with previously scanned paths scanned again when `bForceRescan` is true. The primary call will use:

```cpp
AssetRegistry.ScanPathsSynchronous({ TEXT("/Game") }, true, false);
```

No filesystem pre-enumeration is needed.

## Physical Content and virtual mount paths

- `<Project>/Content` is the physical filesystem directory.
- `/Game` is Unreal's mounted long-package root for project content.
- `/Engine` is a separate mount and must not be scanned.
- Plugin content uses separate plugin mount roots and is outside `/Game`; the plugin also declares `CanContainContent: false`.

A recursive `/Game` scan covers newly copied subfolders below the project Content directory.

## Content Browser refresh behavior

UE 5.7's `UContentBrowserAssetDataSource::Initialize()` subscribes directly to:

- `IAssetRegistry::OnAssetsAdded()`;
- `OnAssetRemoved()`;
- `OnAssetRenamed()`;
- `OnAssetUpdated()`;
- `OnAssetUpdatedOnDisk()`;
- `OnPathsAdded()`;
- `OnPathsRemoved()`.

`OnAssetsAdded()` queues added-item updates and marks owning paths populated. Therefore Asset Registry scan events are the supported refresh path for newly discovered assets and folders. An explicit Content Browser selection/sync operation is not required and would undesirably move focus/selection.

No stable public “refresh every asset view” call exists on `IContentBrowserSingleton`; the internal `RequestSlowFullListRefresh()` path is private. It will not be used.

## Refresh Current Folder feasibility

UE 5.7 publicly exposes `IContentBrowserSingleton::GetCurrentPath()`, returning `FContentBrowserItemPath`. Its implementation constructs the item path from the current virtual path and uses `IContentBrowserDataModule` to resolve an internal package path. When `HasInternalPath()` is true, `GetInternalPathString()` yields an Asset Registry-compatible path such as `/Game/Environment`.

This is a stable public API and avoids the brittle `GetSelectedPathViewFolders()` result, which source inspection shows returns virtual paths such as `/All/Game/...`.

Decision: implement `Refresh Current Folder`, but accept only `/Game` or descendants of `/Game/`. Reject empty, virtual-only, `/Engine`, and plugin paths.

## Behavior and failure model

- Valid `.uasset` and `.umap` packages supported by UE 5.7 are discoverable through the Asset Registry scan.
- Unsupported non-package files are ignored by Asset Registry discovery.
- Corrupt packages, newer-version packages, missing custom classes/plugins, missing references, and redirectors are handled by Unreal's package/registry pipeline and logged by the engine. The command must not attempt repair.
- `ScanPathsSynchronous` returns `void`; “completed” means the scan call completed, not that every external file was valid or loadable.
- A changed package that is already loaded is not safely hot-reloaded by this operation. The ticket targets newly copied packages.
- Large `/Game` trees can block the editor while the synchronous scan runs. The exact supported API is synchronous, so the implementation will favor correctness and avoid unsafe background use of editor/registry APIs.

## Re-entrancy and lifecycle

- Module-owned `bIsRefreshingContent` will gate both commands.
- Command `CanExecute` delegates will disable them during a scan.
- `TGuardValue<bool>` will restore the flag on every normal return path.
- The operation is synchronous on the editor thread; no delegate handle or asynchronous callback lifetime is introduced.
- Shutdown cleanup remains symmetrical because there are no new persistent subscriptions or tab spawners.

## Notifications and logging

Use `FSlateNotificationManager` / `SNotificationItem` for concise start, success, and failure feedback. Log:

- scan start and virtual path;
- elapsed seconds;
- module/path validation failures;
- completion.

Do not report asset counts because the scan API does not return a reliable cheap change count.

## Tests and validation

Existing tests are colocated in `Source/LandscapeHeightmapTracker/Private/Tests` and use Editor-context automation flags. There are no command/menu tests.

Add pure tests for project-content path acceptance/rejection. Command registration and actual external-copy discovery remain editor integration scenarios.

Documented build command:

```text
C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat UE57EditorEditor Win64 Development -Project="J:/Projects/UE_Projects/UE57Editor/UE57Editor.uproject"
```

Validation order:

1. targeted Refresh Content automation tests;
2. all `LandscapeHeightmapTracker.*` automation tests where available;
3. UE57Editor Editor target build;
4. manual editor scenario if an interactive running editor is available.

## Smallest coherent change

- Add two UI commands to the existing command set.
- Map both through the existing module command list.
- Add both entries to the existing Tools menu section.
- Add a small private path-validation helper and automation tests.
- Add only required module dependencies: `AssetRegistry`, `ContentBrowser`, and `ContentBrowserData`.
- Add user documentation and test-plan scenarios.

No new window, toolbar framework, background thread, package modification, explicit browser selection change, asset repair, or broad mount scan is needed.
