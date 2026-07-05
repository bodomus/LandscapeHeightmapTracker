#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LandscapeTrackerSettings.generated.h"

UCLASS(Config=EditorPerProjectUserSettings)
class LANDSCAPEHEIGHTMAPTRACKER_API ULandscapeTrackerSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config)
	FString LastHeightmapDirectory;

	UPROPERTY(Config)
	FString LastHeightmapPath;

	UPROPERTY(Config)
	bool bFlipX = false;

	UPROPERTY(Config)
	bool bFlipY = true;

	UPROPERTY(Config)
	bool bTrackClicks = false;
};
