# Installation

1. Copy `LandscapeHeightmapTracker` to:

```text
<MyProject>/Plugins/LandscapeHeightmapTracker
```

2. Close Unreal Editor.
3. Regenerate project files if your project uses generated IDE files.
4. Build the project Editor target:

```text
<UE_5.7>/Engine/Build/BatchFiles/Build.bat <YourProjectEditorTarget> Win64 Development -Project="<MyProject>/<MyProject>.uproject"
```

5. Launch Unreal Editor.
6. Enable `Landscape Heightmap Tracker` in the Plugins window if needed.
7. Open `Tools -> Landscape Heightmap Tracker`.

For plugin-only validation, you can also place this repository under a test project's `Plugins` folder and build that project's Editor target.

## Automatic ZIP package

Every successful Win64 build creates an installable archive at:

```text
<PluginDir>/build/LandscapeHeightmapTracker-Win64-<Configuration>.zip
```

This is an UnrealBuildTool post-build step, so it behaves the same when the build
is started from Rider, Visual Studio, another editor, or `Build.bat`. Extract the
archive into another Unreal project's `Plugins` directory; the archive already
contains the top-level `LandscapeHeightmapTracker` folder.
