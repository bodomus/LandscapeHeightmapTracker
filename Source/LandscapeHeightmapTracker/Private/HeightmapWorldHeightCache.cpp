#include "HeightmapWorldHeightCache.h"

#include "LandscapeDataAccess.h"

FHeightmapWorldHeightData FHeightmapWorldHeightCache::Build(
	const TArray64<uint8>& RawGrayscaleData,
	int32 BitDepth,
	const FIntPoint& ImageSize,
	const FLandscapeTrackerBounds& LocalBounds,
	const FTransform& LandscapeTransform,
	const FLandscapeTrackerMappingOptions& MappingOptions,
	FString& OutError)
{
	FHeightmapWorldHeightData Result;
	if ((BitDepth != 8 && BitDepth != 16) || ImageSize.X < 2 || ImageSize.Y < 2)
	{
		OutError = TEXT("Height data requires an 8-bit or 16-bit image of at least 2 x 2 pixels.");
		return Result;
	}

	if (!LocalBounds.IsValid())
	{
		OutError = TEXT("Landscape bounds are unavailable.");
		return Result;
	}

	const int64 SampleCount = static_cast<int64>(ImageSize.X) * ImageSize.Y;
	const int32 BytesPerSample = BitDepth / 8;
	if (SampleCount > MAX_int32 || RawGrayscaleData.Num() != SampleCount * BytesPerSample)
	{
		OutError = TEXT("Decoded heightmap data does not match the image dimensions.");
		return Result;
	}

	Result.RawHeights.SetNumUninitialized(static_cast<int32>(SampleCount));
	Result.HeightMeters.SetNumUninitialized(static_cast<int32>(SampleCount));
	Result.MinRawHeight = MAX_uint16;
	Result.MaxRawHeight = 0;
	Result.MinHeightMeters = TNumericLimits<float>::Max();
	Result.MaxHeightMeters = TNumericLimits<float>::Lowest();

	for (int32 Y = 0; Y < ImageSize.Y; ++Y)
	{
		for (int32 X = 0; X < ImageSize.X; ++X)
		{
			const int32 SampleIndex = X + Y * ImageSize.X;
			const FVector2D DisplayUV(
				static_cast<double>(X) / (ImageSize.X - 1),
				static_cast<double>(Y) / (ImageSize.Y - 1));
			const FLandscapeTrackerReverseMappingResult Mapping =
				FLandscapeCoordinateMapper::MapUVToLocalPosition(LocalBounds, DisplayUV, MappingOptions);
			if (!Mapping.bIsValid)
			{
				OutError = Mapping.FailureReason;
				return FHeightmapWorldHeightData();
			}

			const uint16 HeightValue = ExpandSampleToLandscapeHeight(
				RawGrayscaleData.GetData() + static_cast<int64>(SampleIndex) * BytesPerSample,
				BitDepth);
			const float HeightMeters = static_cast<float>(LandscapeHeightToWorldMeters(
				HeightValue,
				FVector2D(Mapping.LocalPosition.X, Mapping.LocalPosition.Y),
				LandscapeTransform));
			Result.RawHeights[SampleIndex] = HeightValue;
			Result.HeightMeters[SampleIndex] = HeightMeters;
			Result.MinRawHeight = FMath::Min(Result.MinRawHeight, HeightValue);
			Result.MaxRawHeight = FMath::Max(Result.MaxRawHeight, HeightValue);
			Result.MinHeightMeters = FMath::Min(Result.MinHeightMeters, HeightMeters);
			Result.MaxHeightMeters = FMath::Max(Result.MaxHeightMeters, HeightMeters);
		}
	}

	Result.bIsValid = true;
	return Result;
}

uint16 FHeightmapWorldHeightCache::ExpandSampleToLandscapeHeight(const uint8* SampleData, int32 BitDepth)
{
	if (BitDepth == 8)
	{
		return static_cast<uint16>(SampleData[0]) * 257u;
	}

	uint16 Value = 0;
	FMemory::Memcpy(&Value, SampleData, sizeof(Value));
	return Value;
}

double FHeightmapWorldHeightCache::NormalizeLandscapeHeight(uint16 LandscapeHeight)
{
	return static_cast<double>(LandscapeHeight) / MAX_uint16;
}

double FHeightmapWorldHeightCache::LandscapeHeightToLocalZ(uint16 LandscapeHeight)
{
	return LandscapeDataAccess::GetLocalHeight(LandscapeHeight);
}

double FHeightmapWorldHeightCache::LandscapeHeightToWorldMeters(
	uint16 LandscapeHeight,
	const FVector2D& LocalXY,
	const FTransform& LandscapeTransform)
{
	const double LocalZ = LandscapeHeightToLocalZ(LandscapeHeight);
	const FVector WorldPosition = LandscapeTransform.TransformPosition(FVector(LocalXY.X, LocalXY.Y, LocalZ));
	return WorldPosition.Z / 100.0;
}

