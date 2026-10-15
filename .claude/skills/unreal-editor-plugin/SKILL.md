---
name: unreal-editor-plugin
description: Unreal Engine 5.7 C++ editor-plugin know-how for this repo - Slate UI, editor modes, delegates and lifetime, transactions/Undo, editor-only actors, automation tests, and how to build and run tests with Scripts/Invoke-Validation.ps1. Use whenever writing, changing or reviewing C++ in Source/.
---

# Unreal editor plugin (UE 5.7, Win64)

## Orientation
- One `Editor` module: `Source/LandscapeHeightmapTracker`. Startup/shutdown in
  `LandscapeHeightmapTrackerModule.cpp`; UI in `SLandscapeHeightmapTrackerPanel.cpp`;
  viewport input via `FLandscapeHeightmapTrackerEdMode`; pure logic in mapper /
  generator / placement classes; tests in `Private/Tests/`.
- Read `Docs/ARCHITECTURE.md` first. Engine headers: `$LHT_UE_ROOT/Engine/Source/...`
  (authoritative for API signatures — prefer them over memory or Context7).

## Rules that reviewers check
**Lifetime**
- UObject refs in non-UObject classes: `TWeakObjectPtr` (or `TStrongObjectPtr`/
  `FGCObject` when ownership is intended). Check `IsValid()` before use.
- Every `Add*`/`Register*` in `StartupModule` or `Construct` has a matching
  `Remove`/`Unregister` in `ShutdownModule`/destructor. Store `FDelegateHandle`s.
- Slate: bind with `SP`/`BindSP` to `SharedThis`; no raw `this` in lambdas that can
  outlive the widget; widgets owned via `TSharedPtr`/`SAssignNew`.

**Editor mutations**
- `FScopedTransaction` + `Modify()` on every object before changing it; Undo/Redo must work.
- Do not dirty packages/levels when nothing changed (early-out before the transaction).
- Generated helper actors: editor-only (`bIsEditorOnlyActor`, `SetIsTemporarilyHiddenInEditor` as needed),
  `NoCollision`, outliner folder, stable label, tag/metadata for identification.
- UObject creation/modification on the game thread only; no heavy work in
  `OnPaint`/`Tick`. Handle missing Landscape, level change and World Partition safely.

**Code style**
- UE naming (`F/U/A/S/E/I/T` prefixes), `check`/`ensure` for invariants,
  `UE_LOG` with a file-local category (`DEFINE_LOG_CATEGORY_STATIC(Log<Area>, Log, All)`,
  as existing files do), `LOCTEXT` for UI text.
- New module dependencies go into `LandscapeHeightmapTracker.Build.cs` (Private unless
  exposed in Public headers). Keep includes minimal (IWYU).

## Tests
- `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FName, "LandscapeHeightmapTracker.<Area>.<Case>",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)`.
- Keep math/placement/mapping testable without an editor world; editor-world tests
  (create/update/delete, Undo/Redo) use a transient world or the editor world carefully
  and clean up.
- Assert behavior (values, counts, transforms), not just "did not crash".

## Build & test (Windows only)
Env vars once: `setx LHT_UE_ROOT "C:\Program Files\Epic Games\UE_5.7"` and
`setx LHT_HOST_UPROJECT "J:\...\UE57Editor.uproject"`.

```
# build host editor target + run all plugin tests (NullRHI)
powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Invoke-Validation.ps1
# narrow filter
... -File Scripts/Invoke-Validation.ps1 -TestFilter "LandscapeHeightmapTracker.ContourBoxes"
# UI / screenshot tests
... -File Scripts/Invoke-Validation.ps1 -TestFilter "LandscapeHeightmapTracker.UI." -Rhi Offscreen
# build only / tests only
... -SkipTests     ... -SkipBuild
```
The script prints exact commands, writes logs/reports to `Saved/Validation/<timestamp>/`
and exits non-zero on build failure or any failed test. Quote its summary in reports.
Close the Unreal Editor holding the host project before building (Live Coding locks DLLs).
