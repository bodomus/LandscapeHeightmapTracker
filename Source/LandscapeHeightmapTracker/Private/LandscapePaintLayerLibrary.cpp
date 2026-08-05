#include "LandscapePaintLayerLibrary.h"

#include "LandscapePaintLayerService.h"
#include "LandscapeProxy.h"

TArray<FLandscapePaintTargetLayerInfo> ULandscapePaintLayerLibrary::GetSelectedLandscapePaintTargetLayers()
{
	TArray<FLandscapePaintTargetLayerInfo> Layers;
	FString Error;
	if (ALandscapeProxy* Landscape = FLandscapePaintLayerService::GetExactlyOneSelectedLandscape(Error))
	{
		FLandscapePaintLayerService::GetTargetLayers(Landscape, Layers, Error);
	}
	return Layers;
}

FLandscapePaintLayerRemoveResult ULandscapePaintLayerLibrary::RemoveSelectedLandscapePaintTargetLayers(
	const TArray<FName>& LayerNames,
	bool bConfirm)
{
	FLandscapePaintLayerRemoveResult Result;
	if (!bConfirm)
	{
		Result.Message = TEXT("Removal was not confirmed. Pass bConfirm=true only after reviewing the target layer names.");
		return Result;
	}

	FString Error;
	ALandscapeProxy* Landscape = FLandscapePaintLayerService::GetExactlyOneSelectedLandscape(Error);
	if (!Landscape)
	{
		Result.Message = Error;
		return Result;
	}
	return FLandscapePaintLayerService::RemoveTargetLayers(Landscape, LayerNames);
}

FLandscapePaintLayerRemoveResult ULandscapePaintLayerLibrary::RemoveAllSelectedLandscapePaintTargetLayers(bool bConfirm)
{
	FLandscapePaintLayerRemoveResult Result;
	if (!bConfirm)
	{
		Result.Message = TEXT("Remove All was not confirmed. Pass bConfirm=true only after accepting permanent paint-data loss.");
		return Result;
	}

	FString Error;
	ALandscapeProxy* Landscape = FLandscapePaintLayerService::GetExactlyOneSelectedLandscape(Error);
	if (!Landscape)
	{
		Result.Message = Error;
		return Result;
	}

	TArray<FLandscapePaintTargetLayerInfo> Layers;
	if (!FLandscapePaintLayerService::GetTargetLayers(Landscape, Layers, Error))
	{
		Result.Message = Error;
		return Result;
	}

	TArray<FName> LayerNames;
	LayerNames.Reserve(Layers.Num());
	for (const FLandscapePaintTargetLayerInfo& Layer : Layers)
	{
		LayerNames.Add(Layer.LayerName);
	}
	return FLandscapePaintLayerService::RemoveTargetLayers(Landscape, LayerNames);
}
