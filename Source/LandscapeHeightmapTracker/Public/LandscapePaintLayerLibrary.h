#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LandscapePaintLayerTypes.h"
#include "LandscapePaintLayerLibrary.generated.h"

UCLASS()
class LANDSCAPEHEIGHTMAPTRACKER_API ULandscapePaintLayerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UE57Editor|Landscape Paint Layers")
	static TArray<FLandscapePaintTargetLayerInfo> GetSelectedLandscapePaintTargetLayers();

	UFUNCTION(BlueprintCallable, Category = "UE57Editor|Landscape Paint Layers")
	static FLandscapePaintLayerRemoveResult RemoveSelectedLandscapePaintTargetLayers(
		const TArray<FName>& LayerNames,
		bool bConfirm);

	UFUNCTION(BlueprintCallable, Category = "UE57Editor|Landscape Paint Layers")
	static FLandscapePaintLayerRemoveResult RemoveAllSelectedLandscapePaintTargetLayers(bool bConfirm);
};
