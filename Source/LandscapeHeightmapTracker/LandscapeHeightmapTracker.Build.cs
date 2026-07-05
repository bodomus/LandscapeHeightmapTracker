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
				"EditorFramework",
				"DesktopPlatform",
				"ImageWrapper",
				"InputCore",
				"LevelEditor",
				"PropertyEditor",
				"Projects",
				"ToolMenus",
				"UnrealEd"
			});
	}
}
