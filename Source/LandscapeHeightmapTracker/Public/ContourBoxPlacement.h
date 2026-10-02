#pragma once

#include "CoreMinimal.h"

struct FContourBoxSettings
{
	double BoxLengthMeters = 10.0;
	double BoxHeightMeters = 10.0;
	double BoxThicknessMeters = 2.0;
	double GapLengthMeters = 10.0;
	double ZOffsetMeters = 0.0;
	int32 MaxInstances = 10000;
};

struct FContourBoxPlacementResult
{
	bool bIsValid = false;
	FString Error;
	double PolylineLengthMeters = 0.0;
	TArray<FTransform> Transforms;
};

class LANDSCAPEHEIGHTMAPTRACKER_API FContourBoxPlacement
{
public:
	static FContourBoxPlacementResult Generate(
		const TArray<FVector>& WorldPoints,
		bool bClosed,
		double ContourWorldZMeters,
		const FContourBoxSettings& Settings);
};
