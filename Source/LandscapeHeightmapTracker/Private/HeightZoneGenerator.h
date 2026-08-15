#pragma once

#include "CoreMinimal.h"
#include "HeightZoneTypes.h"

class FHeightZoneGenerator
{
public:
	static bool DoesRangeOverlap(
		double HeightAMeters,
		double HeightBMeters,
		double AvailableMinHeightMeters,
		double AvailableMaxHeightMeters);

	static FHeightZoneResult Generate(
		const TArray<float>& HeightMeters,
		int32 Width,
		int32 Height,
		double TargetHeightMeters,
		EHeightZoneMode Mode);

	static FHeightZoneResult Generate(
		const TArray<float>& HeightMeters,
		int32 Width,
		int32 Height,
		double HeightAMeters,
		double HeightBMeters,
		EHeightZoneMode Mode);
};

