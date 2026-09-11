# ScanVault Importer v1 Investigation

## Ticket

UE5-19: add a UE57Editor consumer for ScanVault schema v1 manifests (`*.scanvault-ue.json`).

## Baseline

- Repository root: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`
- Initial branch: `main`
- Working branch: `codex/UE5-19-scanvault-importer-v1`
- Initial commit: `854dbea16b2e075214aa7612b391e13cc4e6abde`
- Initial working tree: clean
- `master`/`origin/master`: absent in this clone, so the branch was created from `main`.

## Workflow Level

Level 2. The ticket adds a new editor workflow, new importer subsystem, ToolMenus entry, Slate preview, Unreal asset creation/import, docs, and tests.

## Source Contract

The source contract was read from `bodomus/Megascans-Library-Viewer:Docs/unreal-import-package.md`.

Key contract points:

- `schemaVersion` is explicit and v1 is the only supported version.
- ScanVault owns asset identification, primary mesh and LOD selection, texture semantic roles, destination suggestion, material profile snapshot, options, and validation state.
- UE57Editor owns Unreal API calls, asset import, texture setup, mesh/LOD handling, master material resolution, MI creation, parameter assignment, Nanite, save, cleanup, and report.
- Mapping presence is authoritative: present mappings are assigned, absent mappings are not recreated.
- Source files and manifest are read-only.

## Graphify

- Availability: `graphify --help` succeeded.
- Existing graph: `graphify-out/graph.json` and report artifacts are present.
- Health check: `graphify check-update .` produced no required-update output.
- Queries:
  - `graphify query "ScanVault importer entry points commands menu UI settings import helpers JSON AssetTools Unreal plugin architecture" --budget 3000`
  - `graphify explain "FLandscapeHeightmapTrackerModule::RegisterMenus()"`
  - `graphify query "Which files own command registration ToolMenus menu entries and tests for LandscapeHeightmapTracker" --budget 2500`

Graphify identified the module/menu/command surface and existing tests as likely integration points. These were validated in source.

## CRG

- Availability: `code-review-graph --help` succeeded.
- Previous graph was stale: it was built on `codex/ue5-18-refresh-content`.
- Refreshed with `code-review-graph build --repo .`.
- Refreshed status: 284 nodes, 2478 edges, 51 files, built on `codex/UE5-19-scanvault-importer-v1` at commit `854dbea16b2e`.
- `code-review-graph detect-changes --brief` reported only existing worktree deltas after ticket artifact creation and did not replace source-level validation.

## Source Validation

Validated files:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerCommands.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerCommands.cpp`
- `Source/LandscapeHeightmapTracker/LandscapeHeightmapTracker.Build.cs`
- `Source/LandscapeHeightmapTracker/Private/Tests/ContentRefreshPathUtilsTests.cpp`
- `Docs/ARCHITECTURE.md`
- Installed UE 5.7 headers for `UAssetImportTask`, `IAssetTools`, `UFbxImportUI`, `UFbxStaticMeshImportData`, `UStaticMeshEditorSubsystem`, `FbxMeshUtils`, `UMaterialInstanceConstantFactoryNew`, `UMaterialEditingLibrary`, and `UStaticMesh` Nanite settings.

## Current Behavior

The plugin registers three editor commands: opening the Landscape Heightmap Tracker tab, refreshing all `/Game`, and refreshing the current Content Browser folder. There is no ScanVault import command, no manifest parser, no package preview, no UE asset import flow, and no ScanVault documentation.

## Expected Behavior

The plugin should expose a `Tools -> Landscape Heightmap Tracker -> ScanVault Import Package` command, open a manifest file picker, parse and validate schema v1, show a preview, import declared files only, create/assign a Material Instance according to active mappings, save assets, and show a deterministic report.

## Main Implementation Gap

All ScanVault-specific consumer logic is missing. Existing code provides a suitable module/menu command pattern and test style but no general import subsystem.

## Important UE 5.7 API Findings

- `IAssetTools::ImportAssetTasks()` and `UAssetImportTask` support automated import with explicit destination and name.
- `UFbxImportUI`/`UFbxStaticMeshImportData` expose static mesh import options, including Nanite build.
- `UStaticMeshEditorSubsystem::ImportLOD()` is the public editor subsystem entry point for importing custom static mesh LODs.
- `UMaterialInstanceConstantFactoryNew` supports creating a `UMaterialInstanceConstant` with `InitialParent`.
- `UMaterialInterface::GetAllTextureParameterInfo()` can verify texture parameter names before assignment.
- `UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue()` can assign editor MI texture parameters.
- `UStaticMesh::GetNaniteSettings()`/`SetNaniteSettings()` are available in UE 5.7.

## Tests

Existing automation tests are colocated in `Source/LandscapeHeightmapTracker/Private/Tests` and use `WITH_DEV_AUTOMATION_TESTS`, `IMPLEMENT_SIMPLE_AUTOMATION_TEST`, `EditorContext`, and `EngineFilter`.

Practical first coverage:

- manifest parse/schema behavior;
- destination path policy;
- deterministic naming;
- source validation and LOD policy;
- texture role settings policy;
- command registration.

Full asset import integration requires a running UE 5.7 editor/project and real ScanVault manifests/assets.

## Risks

- Automated external file import can vary by UE importers and installed Interchange/FBX support.
- UE imports are not transactional; cleanup can only delete assets created in the current import session.
- Multi-slot material assignment needs a conservative policy. v1 should auto-assign only when the mesh has one slot and warn otherwise.
- Manual validation with real Megascans assets is required for final release confidence.
