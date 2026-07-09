#if WITH_DEV_AUTOMATION_TESTS

#include "LandscapeCoordinateMapper.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLandscapeCoordinateMapperBasicTest, "LandscapeHeightmapTracker.Mapper.Basic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapeCoordinateMapperBasicTest::RunTest(const FString& Parameters)
{
	const FLandscapeTrackerBounds Bounds{FVector2D(0.0, 0.0), FVector2D(1000.0, 1000.0)};
	const FIntPoint ImageSize(1009, 1009);
	FLandscapeTrackerMappingOptions Options;
	Options.bFlipY = false;

	const FLandscapeTrackerMappingResult Center = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(500.0, 500.0, 0.0), ImageSize, Options);
	TestTrue(TEXT("Center maps successfully"), Center.bIsValid);
	TestEqual(TEXT("Center U"), Center.NormalizedUV.X, 0.5);
	TestEqual(TEXT("Center V"), Center.NormalizedUV.Y, 0.5);
	TestEqual(TEXT("Center pixel X"), Center.Pixel.X, 504);
	TestEqual(TEXT("Center pixel Y"), Center.Pixel.Y, 504);

	const FLandscapeTrackerMappingResult MinCorner = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(0.0, 0.0, 0.0), ImageSize, Options);
	TestEqual(TEXT("Min corner UV"), MinCorner.NormalizedUV, FVector2D(0.0, 0.0));
	TestEqual(TEXT("Min corner pixel"), MinCorner.Pixel, FIntPoint(0, 0));

	const FLandscapeTrackerMappingResult MaxCorner = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(1000.0, 1000.0, 0.0), ImageSize, Options);
	TestEqual(TEXT("Max corner UV"), MaxCorner.NormalizedUV, FVector2D(1.0, 1.0));
	TestEqual(TEXT("Max corner pixel"), MaxCorner.Pixel, FIntPoint(1008, 1008));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLandscapeCoordinateMapperOptionsTest, "LandscapeHeightmapTracker.Mapper.OptionsAndTransforms", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapeCoordinateMapperOptionsTest::RunTest(const FString& Parameters)
{
	const FLandscapeTrackerBounds Bounds{FVector2D(0.0, 0.0), FVector2D(2000.0, 1000.0)};
	const FIntPoint ImageSize(201, 101);

	FLandscapeTrackerMappingOptions Options;
	Options.bFlipY = false;

	const FLandscapeTrackerMappingResult Rect = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(500.0, 250.0, 0.0), ImageSize, Options);
	TestEqual(TEXT("Non-square U"), Rect.NormalizedUV.X, 0.25);
	TestEqual(TEXT("Non-square V"), Rect.NormalizedUV.Y, 0.25);

	Options.bFlipX = true;
	const FLandscapeTrackerMappingResult FlipX = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(500.0, 250.0, 0.0), ImageSize, Options);
	TestEqual(TEXT("Flip X"), FlipX.NormalizedUV.X, 0.75);

	Options.bFlipX = false;
	Options.bFlipY = true;
	const FLandscapeTrackerMappingResult FlipY = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(500.0, 250.0, 0.0), ImageSize, Options);
	TestEqual(TEXT("Flip Y"), FlipY.NormalizedUV.Y, 0.75);

	Options.bFlipY = false;
	const FTransform Translated(FRotator::ZeroRotator, FVector(10000.0, -3000.0, 0.0), FVector(2.0, 2.0, 1.0));
	const FVector World = Translated.TransformPosition(FVector(1000.0, 500.0, 0.0));
	const FLandscapeTrackerMappingResult Transformed = FLandscapeCoordinateMapper::MapWorldPosition(Translated, Bounds, World, ImageSize, Options);
	TestEqual(TEXT("Translated/scaled U"), Transformed.NormalizedUV.X, 0.5);
	TestEqual(TEXT("Translated/scaled V"), Transformed.NormalizedUV.Y, 0.5);

	const FTransform Rotated(FRotator(0.0, 90.0, 0.0), FVector::ZeroVector, FVector::OneVector);
	const FVector RotatedWorld = Rotated.TransformPosition(FVector(1000.0, 500.0, 0.0));
	const FLandscapeTrackerMappingResult RotatedResult = FLandscapeCoordinateMapper::MapWorldPosition(Rotated, Bounds, RotatedWorld, ImageSize, Options);
	TestEqual(TEXT("Rotated U"), RotatedResult.NormalizedUV.X, 0.5);
	TestEqual(TEXT("Rotated V"), RotatedResult.NormalizedUV.Y, 0.5);

	const FLandscapeTrackerMappingResult Outside = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(-1.0, 500.0, 0.0), ImageSize, Options);
	TestFalse(TEXT("Outside rejects by default"), Outside.bIsValid);

	Options.bClampToBounds = true;
	const FLandscapeTrackerMappingResult Clamped = FLandscapeCoordinateMapper::MapLocalPosition(Bounds, FVector(-1.0, 500.0, 0.0), ImageSize, Options);
	TestTrue(TEXT("Clamp mode maps outside point"), Clamped.bIsValid);
	TestEqual(TEXT("Clamped U"), Clamped.NormalizedUV.X, 0.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLandscapeCoordinateMapperReverseMappingTest, "LandscapeHeightmapTracker.ReverseMapping.UVToLocal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapeCoordinateMapperReverseMappingTest::RunTest(const FString& Parameters)
{
	FLandscapeTrackerMappingOptions Options;
	Options.bFlipY = false;

	const FLandscapeTrackerBounds SquareBounds{FVector2D(0.0, 0.0), FVector2D(1000.0, 1000.0)};
	const FLandscapeTrackerReverseMappingResult Center = FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(0.5, 0.5), Options);
	TestTrue(TEXT("Center maps successfully"), Center.bIsValid);
	TestEqual(TEXT("Center Local X"), Center.LocalPosition.X, 500.0);
	TestEqual(TEXT("Center Local Y"), Center.LocalPosition.Y, 500.0);

	TestEqual(TEXT("Min corner"), FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(0.0, 0.0), Options).LocalPosition, FVector(0.0, 0.0, 0.0));
	TestEqual(TEXT("Max X min Y corner"), FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(1.0, 0.0), Options).LocalPosition, FVector(1000.0, 0.0, 0.0));
	TestEqual(TEXT("Min X max Y corner"), FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(0.0, 1.0), Options).LocalPosition, FVector(0.0, 1000.0, 0.0));
	TestEqual(TEXT("Max corner"), FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(1.0, 1.0), Options).LocalPosition, FVector(1000.0, 1000.0, 0.0));

	Options.bFlipX = true;
	const FLandscapeTrackerReverseMappingResult FlipX = FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(0.25, 0.5), Options);
	TestEqual(TEXT("Flip X Landscape U"), FlipX.LandscapeUV.X, 0.75);
	TestEqual(TEXT("Flip X Local X"), FlipX.LocalPosition.X, 750.0);

	Options.bFlipX = false;
	Options.bFlipY = true;
	const FLandscapeTrackerReverseMappingResult FlipY = FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(0.5, 0.25), Options);
	TestEqual(TEXT("Flip Y Landscape V"), FlipY.LandscapeUV.Y, 0.75);
	TestEqual(TEXT("Flip Y Local Y"), FlipY.LocalPosition.Y, 750.0);

	Options.bFlipY = false;
	const FLandscapeTrackerBounds RectBounds{FVector2D(0.0, 0.0), FVector2D(2000.0, 1000.0)};
	const FLandscapeTrackerReverseMappingResult Rect = FLandscapeCoordinateMapper::MapUVToLocalPosition(RectBounds, FVector2D(0.25, 0.75), Options);
	TestTrue(TEXT("Non-square maps successfully"), Rect.bIsValid);
	TestEqual(TEXT("Non-square Local X"), Rect.LocalPosition.X, 500.0);
	TestEqual(TEXT("Non-square Local Y"), Rect.LocalPosition.Y, 750.0);

	const FLandscapeTrackerBounds InvalidBounds{FVector2D(0.0, 0.0), FVector2D(0.0, 1000.0)};
	TestFalse(TEXT("Invalid bounds rejected"), FLandscapeCoordinateMapper::MapUVToLocalPosition(InvalidBounds, FVector2D(0.5, 0.5), Options).bIsValid);
	TestFalse(TEXT("Outside UV rejected"), FLandscapeCoordinateMapper::MapUVToLocalPosition(SquareBounds, FVector2D(1.25, 0.5), Options).bIsValid);

	return true;
}

#endif
