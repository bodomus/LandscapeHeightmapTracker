#include "HeightZoneGenerator.h"

#include "Algo/Count.h"
#include "Misc/AutomationTest.h"

namespace
{
bool HasOnlyFinitePoints(const FHeightZoneResult& Result)
{
	for (const FHeightContour& Contour : Result.Contours)
	{
		for (const FVector2D& Point : Contour.Points)
		{
			if (!FMath::IsFinite(Point.X) || !FMath::IsFinite(Point.Y))
			{
				return false;
			}
		}
	}
	return true;
}

int32 CountClosedContours(const FHeightZoneResult& Result)
{
	int32 Count = 0;
	for (const FHeightContour& Contour : Result.Contours)
	{
		Count += Contour.bClosed ? 1 : 0;
	}
	return Count;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneFlatMaskTest, "LandscapeHeightmapTracker.HeightZone.FlatMasks", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneFlatMaskTest::RunTest(const FString& Parameters)
{
	const TArray<float> Heights = {10, 10, 10, 10};
	const FHeightZoneResult Above = FHeightZoneGenerator::Generate(Heights, 2, 2, 20.0, EHeightZoneMode::Above);
	const FHeightZoneResult Below = FHeightZoneGenerator::Generate(Heights, 2, 2, 20.0, EHeightZoneMode::Below);
	TestEqual(TEXT("Flat map has no contour"), Above.Contours.Num(), 0);
	TestEqual(TEXT("Above selects no pixels"), static_cast<int32>(Algo::Count(Above.Mask, static_cast<uint8>(255))), 0);
	TestEqual(TEXT("Below selects all pixels"), static_cast<int32>(Algo::Count(Below.Mask, static_cast<uint8>(255))), 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneHillAndPitTest, "LandscapeHeightmapTracker.HeightZone.HillAndPit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneHillAndPitTest::RunTest(const FString& Parameters)
{
	const TArray<float> Hill = {0, 0, 0, 0, 10, 0, 0, 0, 0};
	const TArray<float> Pit = {10, 10, 10, 10, 0, 10, 10, 10, 10};
	const FHeightZoneResult HillResult = FHeightZoneGenerator::Generate(Hill, 3, 3, 5.0, EHeightZoneMode::Above);
	const FHeightZoneResult PitResult = FHeightZoneGenerator::Generate(Pit, 3, 3, 5.0, EHeightZoneMode::Below);
	TestEqual(TEXT("Hill creates one closed contour"), CountClosedContours(HillResult), 1);
	TestEqual(TEXT("Pit creates one closed contour"), CountClosedContours(PitResult), 1);
	TestEqual(TEXT("Hill center is selected"), HillResult.Mask[4], static_cast<uint8>(255));
	TestEqual(TEXT("Pit center is selected below threshold"), PitResult.Mask[4], static_cast<uint8>(255));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneIslandsAndHoleTest, "LandscapeHeightmapTracker.HeightZone.IslandsAndHole", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneIslandsAndHoleTest::RunTest(const FString& Parameters)
{
	TArray<float> Islands;
	Islands.SetNumZeroed(21);
	Islands[1 + 1 * 7] = 10.0f;
	Islands[5 + 1 * 7] = 10.0f;
	const FHeightZoneResult IslandsResult = FHeightZoneGenerator::Generate(Islands, 7, 3, 5.0, EHeightZoneMode::Above);
	TestEqual(TEXT("Two independent islands"), CountClosedContours(IslandsResult), 2);

	TArray<float> Ring;
	Ring.SetNumZeroed(25);
	for (int32 Y = 1; Y <= 3; ++Y)
	{
		for (int32 X = 1; X <= 3; ++X)
		{
			Ring[X + Y * 5] = 10.0f;
		}
	}
	Ring[2 + 2 * 5] = 0.0f;
	const FHeightZoneResult RingResult = FHeightZoneGenerator::Generate(Ring, 5, 5, 5.0, EHeightZoneMode::Above);
	TestEqual(TEXT("Ring has outer and inner contours"), CountClosedContours(RingResult), 2);
	TestEqual(TEXT("Ring hole remains unfilled"), RingResult.Mask[2 + 2 * 5], static_cast<uint8>(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneBoundaryAndSaddleTest, "LandscapeHeightmapTracker.HeightZone.BoundaryAndSaddle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneBoundaryAndSaddleTest::RunTest(const FString& Parameters)
{
	const TArray<float> Boundary = {10, 0, 0, 10, 0, 0, 10, 0, 0};
	const FHeightZoneResult BoundaryResult = FHeightZoneGenerator::Generate(Boundary, 3, 3, 5.0, EHeightZoneMode::ContourOnly);
	TestEqual(TEXT("Boundary contour count"), BoundaryResult.Contours.Num(), 1);
	TestFalse(TEXT("Boundary contour remains open"), BoundaryResult.Contours[0].bClosed);
	TestEqual(TEXT("Contour-only mask remains empty"), static_cast<int32>(Algo::Count(BoundaryResult.Mask, static_cast<uint8>(255))), 0);

	const TArray<float> Saddle = {10, 0, 0, 10};
	const FHeightZoneResult SaddleResult = FHeightZoneGenerator::Generate(Saddle, 2, 2, 5.0, EHeightZoneMode::Above);
	TestEqual(TEXT("Saddle resolves to two segments"), SaddleResult.Contours.Num(), 2);
	TestTrue(TEXT("Saddle coordinates are finite"), HasOnlyFinitePoints(SaddleResult));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneExactAndNegativeTest, "LandscapeHeightmapTracker.HeightZone.ExactAndNegative", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneExactAndNegativeTest::RunTest(const FString& Parameters)
{
	const TArray<float> Exact = {1, 0, 0, -1};
	const FHeightZoneResult ExactResult = FHeightZoneGenerator::Generate(Exact, 2, 2, 0.0, EHeightZoneMode::ContourOnly);
	TestTrue(TEXT("Exact threshold coordinates are finite"), HasOnlyFinitePoints(ExactResult));

	const TArray<float> Negative = {-100, -50, -25, -75};
	const FHeightZoneResult NegativeResult = FHeightZoneGenerator::Generate(Negative, 2, 2, -60.0, EHeightZoneMode::Above);
	TestEqual(TEXT("Negative-height mask is correct"), static_cast<int32>(Algo::Count(NegativeResult.Mask, static_cast<uint8>(255))), 2);
	TestTrue(TEXT("Negative-height contour is finite"), HasOnlyFinitePoints(NegativeResult));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightZoneRangeTest, "LandscapeHeightmapTracker.HeightZone.Range", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightZoneRangeTest::RunTest(const FString& Parameters)
{
	TestTrue(
		TEXT("Partially overlapping range remains applicable"),
		FHeightZoneGenerator::DoesRangeOverlap(100.0, 800.0, -588.728, 588.782));
	TestTrue(
		TEXT("Reversed partially overlapping range remains applicable"),
		FHeightZoneGenerator::DoesRangeOverlap(800.0, 100.0, -588.728, 588.782));
	TestFalse(
		TEXT("Range entirely above available heights is rejected"),
		FHeightZoneGenerator::DoesRangeOverlap(600.0, 800.0, -588.728, 588.782));
	TestFalse(
		TEXT("Range entirely below available heights is rejected"),
		FHeightZoneGenerator::DoesRangeOverlap(-800.0, -600.0, -588.728, 588.782));

	const TArray<float> Heights =
	{
		0, 5, 10,
		0, 5, 10,
		0, 5, 10
	};
	const FHeightZoneResult Forward = FHeightZoneGenerator::Generate(
		Heights, 3, 3, 2.5, 7.5, EHeightZoneMode::Range);
	const FHeightZoneResult Reverse = FHeightZoneGenerator::Generate(
		Heights, 3, 3, 7.5, 2.5, EHeightZoneMode::Range);

	TestEqual(TEXT("Range normalizes minimum"), Forward.MinHeightMeters, 2.5);
	TestEqual(TEXT("Range normalizes maximum"), Forward.MaxHeightMeters, 7.5);
	TestEqual(TEXT("Range selects only middle column"), static_cast<int32>(Algo::Count(Forward.Mask, static_cast<uint8>(255))), 3);
	TestTrue(TEXT("Reversed range has identical mask"), Reverse.Mask == Forward.Mask);
	TestEqual(TEXT("Range produces both boundary contours"), Forward.Contours.Num(), 2);
	TestEqual(TEXT("First contour is tagged with lower boundary"), Forward.Contours[0].BoundaryHeightMeters, 2.5);
	TestEqual(TEXT("Second contour is tagged with upper boundary"), Forward.Contours[1].BoundaryHeightMeters, 7.5);
	TestTrue(TEXT("Range contour coordinates are finite"), HasOnlyFinitePoints(Forward));

	const FHeightZoneResult Equal = FHeightZoneGenerator::Generate(
		Heights, 3, 3, 5.0, 5.0, EHeightZoneMode::Range);
	TestEqual(TEXT("Equal range includes exact threshold"), static_cast<int32>(Algo::Count(Equal.Mask, static_cast<uint8>(255))), 3);
	TestEqual(TEXT("Equal range does not duplicate the boundary contour"), Equal.Contours.Num(), 1);
	return true;
}
