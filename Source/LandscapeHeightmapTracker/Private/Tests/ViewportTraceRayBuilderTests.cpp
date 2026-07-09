#if WITH_DEV_AUTOMATION_TESTS

#include "ViewportTraceRayBuilder.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewportTraceRayBuilderPerspectiveTest, "LandscapeHeightmapTracker.ViewportTrace.Perspective", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FViewportTraceRayBuilderPerspectiveTest::RunTest(const FString& Parameters)
{
	const FVector Origin(100.0, 200.0, 300.0);
	const FVector Direction(10.0, 0.0, 0.0);
	const double TraceDistance = 1000.0;

	const FViewportTraceSegment Segment = FViewportTraceRayBuilder::BuildTraceSegment(true, Origin, Direction, TraceDistance);

	TestTrue(TEXT("Perspective segment is valid"), Segment.bIsValid);
	TestEqual(TEXT("Perspective direction is normalized"), Segment.Direction, FVector(1.0, 0.0, 0.0));
	TestEqual(TEXT("Perspective starts at cursor origin"), Segment.Start, Origin);
	TestEqual(TEXT("Perspective ends forward"), Segment.End, FVector(1100.0, 200.0, 300.0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewportTraceRayBuilderOrthographicTest, "LandscapeHeightmapTracker.ViewportTrace.Orthographic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FViewportTraceRayBuilderOrthographicTest::RunTest(const FString& Parameters)
{
	const FVector Origin(100.0, 200.0, 300.0);
	const FVector Direction(0.0, 0.0, -4.0);
	const double TraceDistance = 1000.0;

	const FViewportTraceSegment Segment = FViewportTraceRayBuilder::BuildTraceSegment(false, Origin, Direction, TraceDistance);

	TestTrue(TEXT("Orthographic segment is valid"), Segment.bIsValid);
	TestEqual(TEXT("Orthographic direction is normalized"), Segment.Direction, FVector(0.0, 0.0, -1.0));
	TestEqual(TEXT("Orthographic starts behind cursor origin"), Segment.Start, FVector(100.0, 200.0, 800.0));
	TestEqual(TEXT("Orthographic ends ahead of cursor origin"), Segment.End, FVector(100.0, 200.0, -200.0));
	TestEqual(TEXT("Orthographic midpoint is cursor origin"), (Segment.Start + Segment.End) * 0.5, Origin);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewportTraceRayBuilderReversedOrthographicTest, "LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FViewportTraceRayBuilderReversedOrthographicTest::RunTest(const FString& Parameters)
{
	const FVector Origin(-50.0, 25.0, 10.0);
	const FVector Direction(0.0, 1.0, 0.0);
	const double TraceDistance = 400.0;

	const FViewportTraceSegment Segment = FViewportTraceRayBuilder::BuildTraceSegment(false, Origin, Direction, TraceDistance);

	TestTrue(TEXT("Reversed orthographic segment is valid"), Segment.bIsValid);
	TestEqual(TEXT("Reversed orthographic starts behind origin"), Segment.Start, FVector(-50.0, -175.0, 10.0));
	TestEqual(TEXT("Reversed orthographic ends ahead of origin"), Segment.End, FVector(-50.0, 225.0, 10.0));
	TestEqual(TEXT("Reversed orthographic midpoint is origin"), (Segment.Start + Segment.End) * 0.5, Origin);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewportTraceRayBuilderInvalidDirectionTest, "LandscapeHeightmapTracker.ViewportTrace.InvalidDirection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FViewportTraceRayBuilderInvalidDirectionTest::RunTest(const FString& Parameters)
{
	const FViewportTraceSegment Segment = FViewportTraceRayBuilder::BuildTraceSegment(false, FVector::ZeroVector, FVector::ZeroVector, 1000.0);

	TestFalse(TEXT("Zero direction is invalid"), Segment.bIsValid);
	TestFalse(TEXT("Invalid direction reports a failure reason"), Segment.FailureReason.IsEmpty());

	return true;
}

#endif
