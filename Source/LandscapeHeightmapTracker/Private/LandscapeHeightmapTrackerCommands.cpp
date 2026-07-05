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
}

#undef LOCTEXT_NAMESPACE
