#include "HeightmapImageInfoAnalyzer.h"

#include "Containers/BitArray.h"

FHeightmapGrayscaleInfo FHeightmapImageInfoAnalyzer::Analyze(const TArray64<uint8>& RawGrayscaleData, int32 BitDepth)
{
	FHeightmapGrayscaleInfo Result;
	if ((BitDepth != 8 && BitDepth != 16) || RawGrayscaleData.IsEmpty())
	{
		return Result;
	}

	const int32 BytesPerSample = BitDepth / 8;
	if (RawGrayscaleData.Num() % BytesPerSample != 0)
	{
		return Result;
	}

	Result.BitDepth = BitDepth;
	Result.PossibleLevelCount = 1 << BitDepth;
	Result.MinValue = MAX_uint16;
	TBitArray<> SeenLevels(false, Result.PossibleLevelCount);

	for (int64 ByteIndex = 0; ByteIndex < RawGrayscaleData.Num(); ByteIndex += BytesPerSample)
	{
		uint16 Value = RawGrayscaleData[ByteIndex];
		if (BitDepth == 16)
		{
			FMemory::Memcpy(&Value, RawGrayscaleData.GetData() + ByteIndex, sizeof(Value));
		}

		Result.MinValue = FMath::Min(Result.MinValue, Value);
		Result.MaxValue = FMath::Max(Result.MaxValue, Value);
		if (!SeenLevels[Value])
		{
			SeenLevels[Value] = true;
			++Result.UniqueLevelCount;
		}
	}

	Result.bIsValid = true;
	return Result;
}

