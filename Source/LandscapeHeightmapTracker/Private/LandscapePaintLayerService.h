#pragma once

#include "CoreMinimal.h"
#include "LandscapePaintLayerTypes.h"

class ALandscapeProxy;

class FLandscapePaintLayerService
{
public:
	static ALandscapeProxy* GetExactlyOneSelectedLandscape(FString& OutError);
	static bool GetTargetLayers(
		ALandscapeProxy* Landscape,
		TArray<FLandscapePaintTargetLayerInfo>& OutLayers,
		FString& OutError);
	static bool ValidateLayerNames(
		const TArray<FLandscapePaintTargetLayerInfo>& AvailableLayers,
		const TArray<FName>& RequestedLayerNames,
		TArray<FName>& OutValidatedLayerNames,
		FString& OutError);
	static FLandscapePaintLayerRemoveResult RemoveTargetLayers(
		ALandscapeProxy* Landscape,
		const TArray<FName>& LayerNames);
};
