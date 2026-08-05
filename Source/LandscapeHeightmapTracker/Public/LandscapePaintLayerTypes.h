#pragma once

#include "CoreMinimal.h"
#include "LandscapePaintLayerTypes.generated.h"

class ULandscapeLayerInfoObject;

USTRUCT(BlueprintType)
struct LANDSCAPEHEIGHTMAPTRACKER_API FLandscapePaintTargetLayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	FName LayerName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	TObjectPtr<ULandscapeLayerInfoObject> LayerInfo = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	FString LayerInfoPath;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	FString BlendType;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	bool bExistsInLandscapeMaterial = false;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	bool bIsOrphaned = false;
};

USTRUCT(BlueprintType)
struct LANDSCAPEHEIGHTMAPTRACKER_API FLandscapePaintLayerRemoveResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	int32 RemovedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	TArray<FName> RemovedLayers;

	UPROPERTY(BlueprintReadOnly, Category = "Landscape Paint Layers")
	FString Message;
};
