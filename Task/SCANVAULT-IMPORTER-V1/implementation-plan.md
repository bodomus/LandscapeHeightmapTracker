# ScanVault Importer v1 Implementation Plan

## Scope

Implement a first complete UE57Editor consumer for ScanVault schema v1 without adding ScanVault launch, IPC, watching, batch import, source conversion, material graph generation, or automatic Master Material creation.

## Architecture

Add a small ScanVault subsystem under `Source/LandscapeHeightmapTracker/Private/ScanVault/`:

- `ScanVaultImportTypes`: manifest structures, report structures, naming and policy helpers.
- `ScanVaultManifestReader`: UTF-8 JSON parsing and schema v1 structure validation.
- `ScanVaultImporter`: Unreal asset import, texture configuration, mesh/LOD handling, material instance handling, save, report, and cleanup.
- `SScanVaultImportWindow`: Slate preview and confirmation UI.

Wire this into existing editor infrastructure:

- add `ScanVault Import Package` command to `FLandscapeHeightmapTrackerCommands`;
- map it in `FLandscapeHeightmapTrackerModule::StartupModule()`;
- add it to `Tools -> Landscape Heightmap Tracker`;
- keep the module handler thin: file picker, read/validate, preview, call importer.

## Policies

- Schema:
  - support only `schemaVersion == 1`;
  - reject missing, invalid, or newer schemas.
- Source files:
  - validate existence, readability, not directory, and supported extensions;
  - do not search adjacent files;
  - never modify source paths or manifest.
- Destination:
  - accept only valid `/Game/...` long package paths;
  - block invalid paths;
  - do not redirect silently.
- Conflicts:
  - default v1 UI uses `Cancel`;
  - `Import with unique suffix` is available and uses `IAssetTools::CreateUniqueAssetName`;
  - no silent overwrite.
- Textures:
  - trust manifest roles;
  - apply role-based sRGB/compression;
  - warn when role is `roughness` and `mapType` is `gloss`.
- Mesh and LODs:
  - import primary mesh from manifest;
  - when `importLods=true`, import declared LODs in numeric order after validating duplicates/gaps;
  - when `importLods=false`, import primary only and report skipped LOD count.
- Material:
  - resolve manifest Master Material through UE object loading;
  - create MI only when requested;
  - assign only active mappings present in manifest;
  - missing material texture parameter is a warning.
- Mesh material slot:
  - one slot: assign MI automatically;
  - more than one slot: warn and leave slot assignments unchanged.
- Cleanup:
  - track assets created in the current session;
  - on failure, delete only session-created assets;
  - report leftovers and `PartialImport` when cleanup is incomplete.

## Tests

Add editor automation tests for pure policy and parser behavior:

- command registration;
- schema v1 parse;
- malformed JSON;
- missing schema;
- newer schema;
- missing packageId/destination;
- invalid role;
- destination path validation;
- deterministic names;
- LOD duplicate/gap validation;
- gloss-origin warning policy.

## Documentation

Create `Docs/ScanVaultImporter.md` covering supported schema, flow, naming, texture settings, LOD/Nanite behavior, conflicts, cleanup limits, troubleshooting, and known limitations.

## Validation

Run in order where available:

1. `git diff --check`
2. UE automation tests for `LandscapeHeightmapTracker.ScanVault`
3. broader `LandscapeHeightmapTracker.` automation
4. plugin build/package
5. post-change `code-review-graph build --repo .`
6. `code-review-graph detect-changes --brief`
7. Graphify refresh only because this ticket adds a new subsystem and public workflow

Manual UE validation with real ScanVault manifests remains required if no rendering-capable interactive editor session is available.
