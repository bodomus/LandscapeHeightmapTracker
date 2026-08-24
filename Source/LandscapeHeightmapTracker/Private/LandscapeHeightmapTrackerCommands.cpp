#include "LandscapeHeightmapTrackerCommands.h"

#define LOCTEXT_NAMESPACE "FLandscapeHeightmapTrackerCommands"

FLandscapeHeightmapTrackerCommands::FLandscapeHeightmapTrackerCommands()
	: TCommands<FLandscapeHeightmapTrackerCommands>(
		TEXT("LandscapeHeightmapTracker"),
		NSLOCTEXT("Contexts", "LandscapeHeightmapTracker", "Landscape Heightmap Tracker"),
		NAME_None,
		FLandscapeHeightmapTrackerStyle::GetStyleSetName())
{
}

void FLandscapeHeightmapTrackerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Landscape Heightmap Tracker", "Open the Landscape Heightmap Tracker tab.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(
		RefreshContent,
		"Refresh Content",
		"Rescan project Content (/Game) and refresh externally added assets without restarting Unreal Editor.",
		EUserInterfaceActionType::Button,
		FInputChord());
	UI_COMMAND(
		RefreshCurrentFolder,
		"Refresh Current Folder",
		"Rescan the current project Content Browser folder and its subfolders.",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
