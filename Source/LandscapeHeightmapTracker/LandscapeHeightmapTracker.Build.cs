using UnrealBuildTool;

public class LandscapeHeightmapTracker : ModuleRules
{
	public LandscapeHeightmapTracker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Landscape",
				"Slate",
				"SlateCore"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"AssetRegistry",
				"AssetTools",
				"ContentBrowser",
				"ContentBrowserData",
				"EditorFramework",
				"DesktopPlatform",
				"ImageWrapper",
				"InputCore",
				"Json",
				"LevelEditor",
				"MaterialEditor",
				"PropertyEditor",
				"Projects",
				"StaticMeshEditor",
				"ToolMenus",
				"UnrealEd"
			});
	}
}
