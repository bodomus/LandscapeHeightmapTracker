# UE5-18 Implementation Plan — Refresh Content

## Objective

Add safe editor commands that force-rescan project content without restarting Unreal Editor, while preserving the plugin's current Tools-menu architecture and never modifying packages.

## Planned changes

1. **Path policy helper**
   - Add a small private helper that accepts only `/Game` and descendants of `/Game/`.
   - Keep path policy independent from Slate and Asset Registry so it can be automation-tested.

2. **Command declarations**
   - Add `RefreshContent` with no default shortcut.
   - Add `RefreshCurrentFolder` with no default shortcut.
   - Use ticket-approved labels and concise tooltips.

3. **Module execution**
   - Map `RefreshContent` to a forced synchronous scan of `/Game`.
   - Resolve the current Content Browser item path through public UE 5.7 APIs for `RefreshCurrentFolder`.
   - Reject current paths outside `/Game`.
   - Load required modules defensively and report a concise failure if unavailable.
   - Use a module-owned busy flag plus `TGuardValue<bool>`.
   - Measure elapsed time with `FPlatformTime::Seconds()`.
   - Log start, path, failure, and completion.
   - Show one updating Slate notification with success/failure state.

4. **Menu integration**
   - Add both commands to the existing `LevelEditor.MainMenu.Tools` / `LandscapeHeightmapTracker` section.
   - Do not add a new window or global shortcut.

5. **Dependencies**
   - Add private `AssetRegistry` for `IAssetRegistry`.
   - Add private `ContentBrowser` for current-path access.
   - Add private `ContentBrowserData` for `FContentBrowserItemPath` conversion/access.
   - Do not add unrelated candidate modules.

6. **Tests**
   - Add Editor automation tests for valid and invalid project-content paths.
   - Add a UI/command-state test only if it can be expressed without mutating project content or relying on private engine APIs.

7. **Documentation**
   - Add `Docs/RefreshContent.md` covering use, path terminology, safety, limitations, and validation scenarios.
   - Extend `Docs/TEST_PLAN.md` with Refresh Content scenarios.

## Source files expected to change

- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerCommands.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerCommands.cpp`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
- `Source/LandscapeHeightmapTracker/LandscapeHeightmapTracker.Build.cs`
- new private path-policy helper source/header
- new Refresh Content automation test
- `Docs/RefreshContent.md`
- `Docs/TEST_PLAN.md`

## Implementation constraints

- Use the verified UE 5.7 signature:

```cpp
ScanPathsSynchronous(Paths, /* bForceRescan */ true, /* bIgnoreDenyListScanFilters */ false);
```

- Do not scan `/Engine` or plugin mount roots.
- Do not enumerate files before scanning.
- Do not call private Content Browser refresh APIs.
- Do not move Content Browser focus or selection merely to refresh its view.
- Do not create, save, rename, move, delete, resave, repair, or source-control assets.
- Do not invent detection counts.
- Preserve all unrelated working-tree files.

## Validation plan

1. Review the final diff against ticket scope.
2. Update CRG incrementally.
3. Query changed symbols, callers/callees, tests, and impact radius.
4. Run targeted `LandscapeHeightmapTracker.RefreshContent.*` automation tests.
5. Run the broader `LandscapeHeightmapTracker.*` suite when feasible.
6. Build `UE57EditorEditor Win64 Development` with installed UE 5.7.
7. If an interactive editor session is available, validate:
   - command visibility;
   - externally copied compatible `.uasset` and `.umap`;
   - new subfolder discovery;
   - current-folder filtering;
   - busy-state re-enable;
   - invalid/incompatible package handling.
8. Record commands, results, limitations, and remaining risk in `implementation-report.md`.
9. Create `review-UE5-18.md` and update the YouTrack state.
