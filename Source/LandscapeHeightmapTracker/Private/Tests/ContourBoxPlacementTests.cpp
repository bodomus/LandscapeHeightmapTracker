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
	FContourBoxPlacementSeamGapTest,
	"LandscapeHeightmapTracker.ContourBoxes.ClosedSeamGap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementSeamGapTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> Rectangle =
	{
		FVector(0.0, 0.0, 0.0), FVector(1500.0, 0.0, 0.0),
		FVector(1500.0, 1000.0, 0.0), FVector(0.0, 1000.0, 0.0)
	};
	const auto ArcDistance = [](const FVector& Point)
	{
		if (FMath::IsNearlyZero(Point.Y)) { return Point.X; }
		if (FMath::IsNearlyEqual(Point.X, 1500.0)) { return 1500.0 + Point.Y; }
		if (FMath::IsNearlyEqual(Point.Y, 1000.0)) { return 2500.0 + 1500.0 - Point.X; }
		return 4000.0 + 1000.0 - Point.Y;
	};
	FContourBoxSettings Settings;
	for (const double GapMeters : {10.0, 0.0})
	{
		Settings.GapLengthMeters = GapMeters;
		const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(Rectangle, true, 0.0, Settings);
		TestTrue(TEXT("Closed placement is valid"), Result.bIsValid);
		TestEqual(TEXT("Perimeter is fifty meters"), Result.PolylineLengthMeters, 50.0);
		TestEqual(TEXT("One gap per box fits around the loop"), Result.Transforms.Num(), GapMeters == 0.0 ? 5 : 2);
		if (Result.Transforms.IsEmpty()) { return false; }
		for (int32 Index = 0; Index < Result.Transforms.Num(); ++Index)
		{
			const double Center = ArcDistance(Result.Transforms[Index].GetLocation());
			const bool bSeam = Index == Result.Transforms.Num() - 1;
			const double NextCenter = ArcDistance(Result.Transforms[(Index + 1) % Result.Transforms.Num()].GetLocation())
				+ (bSeam ? 5000.0 : 0.0);
			const double ActualGap = NextCenter - Center - Settings.BoxLengthMeters * 100.0;
			TestTrue(bSeam ? TEXT("Last-to-first gap is preserved") : TEXT("Neighbor gap is preserved"),
				ActualGap + KINDA_SMALL_NUMBER >= GapMeters * 100.0);
		}
		TArray<FVector> RepeatedSeam = Rectangle;
		RepeatedSeam.Add(Rectangle[0]);
		const FContourBoxPlacementResult Repeated = FContourBoxPlacement::Generate(RepeatedSeam, true, 0.0, Settings);
		TestEqual(TEXT("Repeated closure point does not add an instance"), Repeated.Transforms.Num(), Result.Transforms.Num());
	}

	const auto ShortLoop = [](double SideCentimeters)
	{
		return TArray<FVector>{FVector::ZeroVector, FVector(SideCentimeters, 0.0, 0.0),
			FVector(SideCentimeters, SideCentimeters, 0.0), FVector(0.0, SideCentimeters, 0.0)};
	};
	Settings.GapLengthMeters = 10.0;
	TestEqual(TEXT("Eight meter closed loop cannot fit a box"),
		FContourBoxPlacement::Generate(ShortLoop(200.0), true, 0.0, Settings).Transforms.Num(), 0);
	TestEqual(TEXT("Fifteen meter loop cannot fit a box and closing gap"),
		FContourBoxPlacement::Generate(ShortLoop(375.0), true, 0.0, Settings).Transforms.Num(), 0);
	TestEqual(TEXT("Twenty meter loop fits one box and one gap"),
		FContourBoxPlacement::Generate(ShortLoop(500.0), true, 0.0, Settings).Transforms.Num(), 1);
	Settings.GapLengthMeters = 0.0;
	TestEqual(TEXT("Fifteen meter loop fits one box with zero required gap"),
		FContourBoxPlacement::Generate(ShortLoop(375.0), true, 0.0, Settings).Transforms.Num(), 1);
	TestEqual(TEXT("Zero gap still rejects a loop shorter than a box"),
		FContourBoxPlacement::Generate(ShortLoop(200.0), true, 0.0, Settings).Transforms.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxPlacementDenseSegmentsTest,
	"LandscapeHeightmapTracker.ContourBoxes.DenseSegments",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxPlacementDenseSegmentsTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> Corners = {FVector::ZeroVector, FVector(5000.0, 0.0, 0.0),
		FVector(5000.0, 5000.0, 0.0), FVector(10000.0, 5000.0, 0.0)};
	TArray<FVector> Dense;
	for (int32 Segment = 0; Segment < Corners.Num() - 1; ++Segment)
	{
		for (int32 Sample = 0; Sample <= 5000; ++Sample)
		{
			Dense.Add(FMath::Lerp(Corners[Segment], Corners[Segment + 1], Sample / 5000.0));
		}
	}
	FContourBoxSettings Settings;
	for (const double Gap : {10.0, 0.0})
	{
		Settings.GapLengthMeters = Gap;
		const FContourBoxPlacementResult Sparse = FContourBoxPlacement::Generate(Corners, false, 100.0, Settings);
		const FContourBoxPlacementResult Result = FContourBoxPlacement::Generate(Dense, false, 100.0, Settings);
		TestTrue(TEXT("Dense segmented contour is valid"), Result.bIsValid);
		TestEqual(TEXT("Sampling across corners preserves count"), Result.Transforms.Num(), Sparse.Transforms.Num());
		TestEqual(TEXT("Complete fifteen-thousand-centimeter contour is sampled"), Result.Transforms.Num(), Gap == 0.0 ? 15 : 8);
		for (int32 Index = 0; Index < FMath::Min(Result.Transforms.Num(), Sparse.Transforms.Num()); ++Index)
		{
			TestTrue(TEXT("Dense and sparse transforms agree"), Result.Transforms[Index].Equals(Sparse.Transforms[Index], 0.001));
		}
	}
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
