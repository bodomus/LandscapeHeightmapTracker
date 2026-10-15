#pragma once

#include "ContourBoxPlacement.h"
#include "HeightZoneTypes.h"
#include "LandscapeCoordinateMapper.h"

class ALandscapeProxy;
class ALHTContourBoxesActor;

class FContourBoxesEditorService
{
public:
	static ALHTContourBoxesActor* FindActor(
		const ALandscapeProxy* Landscape,
		const FGuid& ContourId);

	static bool CreateOrUpdate(
		ALandscapeProxy* Landscape,
		const FLandscapeTrackerBounds& LocalBounds,
		const FLandscapeTrackerMappingOptions& MappingOptions,
		const FHeightContour& Contour,
		const FContourBoxSettings& Settings,
		bool bRequireExistingActor,
		ALHTContourBoxesActor*& OutActor,
		FString& OutError);

	static bool Delete(
		ALandscapeProxy* Landscape,
		const FGuid& ContourId,
		bool& bOutChanged,
		FString& OutError);

private:
	static bool BuildWorldPoints(
		const ALandscapeProxy* Landscape,
		const FLandscapeTrackerBounds& LocalBounds,
		const FLandscapeTrackerMappingOptions& MappingOptions,
		const FHeightContour& Contour,
		TArray<FVector>& OutWorldPoints,
		FString& OutError);
};
