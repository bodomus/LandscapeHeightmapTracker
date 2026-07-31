#include "HeightmapWorldHeightCache.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapWorldHeightConversionTest, "LandscapeHeightmapTracker.HeightZone.WorldHeightConversion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapWorldHeightConversionTest::RunTest(const FString& Parameters)
{
	const uint8 EightBitMaximum = 255;
	TestEqual(
		TEXT("8-bit maximum expands to full Landscape range"),
		FHeightmapWorldHeightCache::ExpandSampleToLandscapeHeight(&EightBitMaximum, 8),
		MAX_uint16);

	const FTransform Transform(
		FRotator::ZeroRotator,
		FVector(0.0, 0.0, 10000.0),
		FVector(1.0, 1.0, 100.0));
	TestEqual(
		TEXT("Midpoint maps to actor Z translation"),
		FHeightmapWorldHeightCache::LandscapeHeightToWorldMeters(32768, FVector2D::ZeroVector, Transform),
		100.0);
	TestEqual(
		TEXT("Landscape Z scale is applied"),
		FHeightmapWorldHeightCache::LandscapeHeightToWorldMeters(32896, FVector2D::ZeroVector, Transform),
		101.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapWorldHeightCacheBuildTest, "LandscapeHeightmapTracker.HeightZone.CacheBuild", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapWorldHeightCacheBuildTest::RunTest(const FString& Parameters)
{
	const TArray<uint16> Source = {32768, 32768, 32768, 32768};
	TArray64<uint8> Raw;
	Raw.SetNumUninitialized(Source.Num() * sizeof(uint16));
	FMemory::Memcpy(Raw.GetData(), Source.GetData(), Raw.Num());

	FLandscapeTrackerBounds Bounds;
	Bounds.Min = FVector2D(0.0, 0.0);
	Bounds.Max = FVector2D(1.0, 1.0);
	FLandscapeTrackerMappingOptions Options;
	Options.bFlipY = false;
	FString Error;
	const FHeightmapWorldHeightData Data = FHeightmapWorldHeightCache::Build(
		Raw,
		16,
		FIntPoint(2, 2),
		Bounds,
		FTransform(FRotator::ZeroRotator, FVector(0.0, 0.0, 5000.0)),
		Options,
		Error);

	TestTrue(TEXT("Cache build succeeds"), Data.bIsValid);
	TestTrue(TEXT("Cache build error is empty"), Error.IsEmpty());
	TestEqual(TEXT("Cache contains every sample"), Data.HeightMeters.Num(), 4);
	TestEqual(TEXT("Minimum height"), Data.MinHeightMeters, 50.0f);
	TestEqual(TEXT("Maximum height"), Data.MaxHeightMeters, 50.0f);
	return true;
}

