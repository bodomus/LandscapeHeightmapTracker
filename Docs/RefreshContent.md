# Refresh Content

## Purpose

`Refresh Content` lets Unreal Editor discover compatible Unreal packages that were copied into the project's Content directory while the editor was already running.

```text
<Project>/Content  = physical filesystem directory
/Game              = mounted Unreal package path for project content
```

The command performs a forced recursive Asset Registry scan of `/Game`. Unreal's Content Browser listens to Asset Registry asset/path events, so newly discovered assets and folders update without changing the current browser selection.

## Usage

1. Copy a compatible asset or package folder into `<Project>/Content`.
2. Return to Unreal Editor.
3. Choose **Tools → Landscape Heightmap Tracker → Refresh Content**.
4. Wait for the completion notification.
5. Newly discovered compatible assets should appear in the Content Browser.

To scan a smaller subtree:

1. Select or navigate to a folder below `/Game` in the primary Content Browser.
2. Choose **Tools → Landscape Heightmap Tracker → Refresh Current Folder**.

`Refresh Current Folder` is enabled only when the current folder resolves to `/Game` or one of its descendants.

## Scope and safety

The commands:

- read package metadata through Unreal's Asset Registry;
- force a recursive scan of the requested project-content path;
- report the scanned virtual path and elapsed time in the Output Log;
- show concise progress and completion feedback.

The commands do **not**:

- scan `/Engine`;
- scan plugin content mounts;
- copy, import, move, rename, delete, save, or resave packages;
- repair dependencies or redirectors;
- change source-control state;
- overwrite assets.

Repeated clicks cannot start overlapping scans. Both commands are disabled while a scan is running and re-enable when the synchronous operation returns.

## Supported results

- Compatible newly copied `.uasset` packages can be discovered.
- Compatible newly copied `.umap` packages can be discovered.
- New subfolders below the scanned path can be discovered.
- Unsupported non-package files are ignored by Asset Registry discovery.

Unreal's Asset Registry and package reader decide whether each file is valid. Engine diagnostics for invalid files are written to the Output Log.

## Limitations

- A package created by a newer/incompatible Unreal version may be rejected.
- Corrupt packages or packages that require missing plugins/classes may not appear.
- Missing referenced assets are not repaired.
- Redirectors are discovered but are not fixed automatically.
- Refreshing does not guarantee safe hot-reload of a package that is already loaded and was modified externally. The intended use is discovery of newly copied content.
- Plugin content is outside `/Game` and is not scanned by these commands.
- A full `/Game` scan is synchronous and may pause the editor in very large projects.
- “Content refresh completed” means the scan call finished; it does not claim that every external file was a valid Unreal package.

## Manual validation

Use test assets created with a compatible Unreal version; do not use paid Marketplace/Fab content in automated validation.

1. Start the editor and confirm a known test asset is absent.
2. Externally copy it to `<Project>/Content/RefreshContentTest/`.
3. Run `Refresh Content`.
4. Confirm the new folder and asset appear and can be selected.
5. Repeat with a compatible `.umap`.
6. Select `/Game/RefreshContentTest`, copy another asset beneath it, and run `Refresh Current Folder`.
7. Verify `/Engine` and plugin paths cannot be scanned through the current-folder command.
8. Review `LogLandscapeHeightmapTracker` and engine Asset Registry/package messages in the Output Log.
