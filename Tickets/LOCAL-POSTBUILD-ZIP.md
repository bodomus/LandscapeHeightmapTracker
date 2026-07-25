# Automatic post-build plugin ZIP

## Goal

After every successful Win64 Unreal build, create an installable ZIP archive of
the `LandscapeHeightmapTracker` plugin in `./build`, regardless of whether the
build was started from Rider, another editor, or the command line.

## Acceptance criteria

- Packaging is attached to UnrealBuildTool rather than an IDE-specific project.
- A successful build creates
  `build/LandscapeHeightmapTracker-<Platform>-<Configuration>.zip`.
- The archive has a top-level `LandscapeHeightmapTracker` directory that can be
  extracted directly into another project's `Plugins` directory.
- The package includes the descriptor, packaging scripts, binaries, source,
  configuration, content, resources, and shaders when those directories exist.
- Generated intermediates, repository metadata, development documentation,
  graph databases, and prior archives are excluded.
- Packaging failure fails the build instead of silently leaving a stale archive.

## Validation

- Validate the plugin descriptor as JSON.
- Run the packaging script directly against the current plugin.
- Inspect ZIP entry names and confirm excluded paths are absent.
- Run a host-project Editor build and confirm its post-build step creates the ZIP.
