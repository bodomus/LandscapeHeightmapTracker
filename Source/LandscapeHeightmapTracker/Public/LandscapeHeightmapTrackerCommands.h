#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LandscapeHeightmapTrackerStyle.h"

class FLandscapeHeightmapTrackerCommands : public TCommands<FLandscapeHeightmapTrackerCommands>
{
public:
	FLandscapeHeightmapTrackerCommands();

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenPluginWindow;
	TSharedPtr<FUICommandInfo> RefreshContent;
	TSharedPtr<FUICommandInfo> RefreshCurrentFolder;
};
