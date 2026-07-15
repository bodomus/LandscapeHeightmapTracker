#pragma once

#include "CoreMinimal.h"

struct FHeightmapGrayscaleInfo
{
	int32 BitDepth = 0;
	int32 PossibleLevelCount = 0;
	int32 UniqueLevelCount = 0;
	uint16 MinValue = 0;
	uint16 MaxValue = 0;
	bool bIsValid = false;
};

class FHeightmapImageInfoAnalyzer
{
public:
	static FHeightmapGrayscaleInfo Analyze(const TArray64<uint8>& RawGrayscaleData, int32 BitDepth);
};

