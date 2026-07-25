# Post-build ZIP investigation

- Workflow level: 2 (shared build-path change).
- The plugin currently has no packaging hook; installation requires copying the
  plugin directory and building the host Editor target.
- UnrealBuildTool 5.7 reads plugin-level `PostBuildSteps` from `.uplugin` files.
- UBT expands `PluginDir`, `TargetPlatform`, and `TargetConfiguration` for plugin
  custom build steps and emits the host-specific batch script.
- Therefore the smallest IDE-independent integration point is the plugin
  descriptor, with packaging logic kept in a versioned script.
- Runtime C++ symbols and Unreal editor lifecycle are not affected.
- The current repository uses Windows/Win64, so the implementation targets the
  confirmed Win64 host command path.
