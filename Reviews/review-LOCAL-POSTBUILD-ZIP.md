# Review — LOCAL-POSTBUILD-ZIP

## Work completed

- Connected ZIP creation to UnrealBuildTool through plugin `PostBuildSteps`.
- Added `Scripts/PackagePlugin.ps1` with strict error handling, isolated temporary
  staging, stale archive replacement, and guaranteed staging cleanup.
- Produced an installable archive with the plugin directory at its root.
- Included the packaging script referenced by the archived descriptor, so the
  post-build step remains valid after installation in another project.
- Filtered generated, repository, graph, build, and IDE-only content.
- Documented output naming and installation.

## Review result

Accepted for the current Win64 project scope. The implementation is independent
of Rider or any other editor because all callers converge on UnrealBuildTool.
Packaging errors propagate as post-build failures, preventing stale output from
being mistaken for a successful package.

## Evidence

- Descriptor parses successfully.
- Direct ZIP content validation passed, with required descriptor/script and
  no excluded paths.
- Full UE 5.7 host target rebuild passed.
- UBT log explicitly recorded `Executing post build script (PostBuild-1.bat)` and
  the archive creation message.
- CRG found no affected runtime flow; Graphify was refreshed after adding the new
  build entry point.

## Remaining limitation

Only Win64 is configured. Cross-host Mac/Linux packaging is outside the confirmed
environment and would need native shell implementations.
