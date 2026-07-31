#pragma once

#include "CoreMinimal.h"
#include "LandscapeCoordinateMapper.h"

struct FHeightmapWorldHeightData
{
	TArray<float> HeightMeters;
	float MinHeightMeters = 0.0f;
	float MaxHeightMeters = 0.0f;
	bool bIsValid = false;
};

class FHeightmapWorldHeightCache
{
public:
	static FHeightmapWorldHeightData Build(
		const TArray64<uint8>& RawGrayscaleData,
		int32 BitDepth,
		const FIntPoint& ImageSize,
		const FLandscapeTrackerBounds& LocalBounds,
		const FTransform& LandscapeTransform,
		const FLandscapeTrackerMappingOptions& MappingOptions,
		FString& OutError);

	static uint16 ExpandSampleToLandscapeHeight(const uint8* SampleData, int32 BitDepth);
	static double LandscapeHeightToWorldMeters(
		uint16 LandscapeHeight,
		const FVector2D& LocalXY,
		const FTransform& LandscapeTransform);
};

