#pragma once

#include "CoreMinimal.h"
#include "HeightZoneTypes.h"

class FHeightRangeGenerator
{
public:
	static FHeightRangeDefinition Normalize(const FHeightRangeDefinition& Range);
	static void NormalizeAndSort(TArray<FHeightRangeDefinition>& Ranges);
	static FHeightRangeValidationResult FindConflict(
		const FHeightRangeDefinition& Candidate,
		const TArray<FHeightRangeDefinition>& ExistingRanges);
	static bool ContainsHeight(
		const FHeightRangeDefinition& Range,
		double HeightMeters,
		bool bIncludeMaximum);
	static FMultiHeightRangeResult Generate(
		const TArray<float>& HeightMeters,
		int32 Width,
		int32 Height,
		const TArray<FHeightRangeDefinition>& Ranges);
};

