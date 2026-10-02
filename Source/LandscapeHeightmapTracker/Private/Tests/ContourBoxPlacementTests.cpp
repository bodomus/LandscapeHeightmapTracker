#include "ContourBoxPlacement.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace
{
TArray<FVector> MakeLine(const FVector& A, const FVector& B)
{
	return {A, B};
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementStepTest,
	"LandscapeHeightmapTracker.ContourBoxes.StepAndStraightLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementStepTest::RunTest(const FString& Parameters)
{
	const FContourBoxSettings Settings;
	const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(
		MakeLine(FVector::ZeroVector, FVector(5000.0, 0.0, 0.0)),
		false,
		100.0,
		Settings);
	TestTrue(TEXT("Placement is valid"), Result.bIsValid);
	TestEqual(TEXT("Fifty meters creates three boxes at a twenty meter step"), Result.Transforms.Num(), 3);
	if (Result.Transforms.Num() == 3)
	{
		TestEqual(TEXT("First center"), Result.Transforms[0].GetLocation(), FVector(500.0, 0.0, 10500.0));
		TestEqual(TEXT("Second center"), Result.Transforms[1].GetLocation(), FVector(2500.0, 0.0, 10500.0));
		TestEqual(TEXT("Third center"), Result.Transforms[2].GetLocation(), FVector(4500.0, 0.0, 10500.0));
		TestEqual(TEXT("Default cube scale is 10 x 2 x 10"), Result.Transforms[0].GetScale3D(), FVector(10.0, 2.0, 10.0));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementOrientationTest,
	"LandscapeHeightmapTracker.ContourBoxes.Orientation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementOrientationTest::RunTest(const FString& Parameters)
{
	FContourBoxSettings Settings;
	Settings.BoxLengthMeters = 10.0;
	Settings.GapLengthMeters = 0.0;
	const auto YawFor = [&Settings](const FVector& End)
	{
		const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(
			MakeLine(FVector::ZeroVector, End), false, 0.0, Settings);
		return Result.Transforms.IsEmpty() ? 999.0 : Result.Transforms[0].Rotator().Yaw;
	};
	TestTrue(TEXT("Horizontal yaw"), FMath::IsNearlyEqual(YawFor(FVector(1000.0, 0.0, 0.0)), 0.0));
	TestTrue(TEXT("Vertical yaw"), FMath::IsNearlyEqual(YawFor(FVector(0.0, 1000.0, 0.0)), 90.0));
	TestTrue(TEXT("Diagonal yaw"), FMath::IsNearlyEqual(YawFor(FVector(1000.0, 1000.0, 0.0)), 45.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementCenterZTest,
	"LandscapeHeightmapTracker.ContourBoxes.CenterZ",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementCenterZTest::RunTest(const FString& Parameters)
{
	FContourBoxSettings Settings;
	Settings.BoxHeightMeters = 6.0;
	Settings.ZOffsetMeters = 2.0;
	const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(
		MakeLine(FVector::ZeroVector, FVector(1000.0, 0.0, 0.0)), false, 100.0, Settings);
	TestEqual(TEXT("Center Z uses contour height, half box height, and offset"), Result.Transforms[0].GetLocation().Z, 10500.0);
	TestTrue(TEXT("Pitch remains zero"), FMath::IsNearlyZero(Result.Transforms[0].Rotator().Pitch));
	TestTrue(TEXT("Roll remains zero"), FMath::IsNearlyZero(Result.Transforms[0].Rotator().Roll));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementClosedAndShortTest,
	"LandscapeHeightmapTracker.ContourBoxes.ClosedAndShort",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementClosedAndShortTest::RunTest(const FString& Parameters)
{
	const FContourBoxSettings Settings;
	const TArray<FVector> ClosedSquare =
	{
		FVector(0.0, 0.0, 0.0),
		FVector(1000.0, 0.0, 0.0),
		FVector(1000.0, 1000.0, 0.0),
		FVector(0.0, 1000.0, 0.0),
		FVector(0.0, 0.0, 0.0)
	};
	const FContourBoxPlacementResult Closed = FContourBoxPlacement::Generate(ClosedSquare, true, 0.0, Settings);
	TestTrue(TEXT("Closed contour is valid"), Closed.bIsValid);
	TestEqual(TEXT("Closed seam does not create a duplicate"), Closed.Transforms.Num(), 2);

	const FContourBoxPlacementResult Short = FContourBoxPlacement::Generate(
		MakeLine(FVector::ZeroVector, FVector(999.0, 0.0, 0.0)), false, 0.0, Settings);
	TestTrue(TEXT("Short contour is valid input"), Short.bIsValid);
	TestEqual(TEXT("Short contour creates no degenerate box"), Short.Transforms.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementLimitTest,
	"LandscapeHeightmapTracker.ContourBoxes.InstanceLimit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementLimitTest::RunTest(const FString& Parameters)
{
	FContourBoxSettings Settings;
	Settings.MaxInstances = 2;
	const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(
		MakeLine(FVector::ZeroVector, FVector(5000.0, 0.0, 0.0)), false, 0.0, Settings);
	TestFalse(TEXT("Excessive instance count is rejected"), Result.bIsValid);
	TestTrue(TEXT("Limit error is actionable"), Result.Error.Contains(TEXT("exceeding the limit")));
	return true;
}

#endif
