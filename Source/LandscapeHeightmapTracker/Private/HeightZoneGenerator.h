#pragma once

#include "CoreMinimal.h"
#include "HeightZoneTypes.h"

class FHeightZoneGenerator
{
public:
	static FHeightZoneResult Generate(
		const TArray<float>& HeightMeters,
		int32 Width,
		int32 Height,
		double TargetHeightMeters,
		EHeightZoneMode Mode);
};

