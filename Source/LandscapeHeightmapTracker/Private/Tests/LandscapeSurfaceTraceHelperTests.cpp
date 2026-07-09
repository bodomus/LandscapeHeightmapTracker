#if WITH_DEV_AUTOMATION_TESTS

#include "LandscapeSurfaceTraceHelper.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLandscapeSurfaceTraceHelperVerticalSegmentTest, "LandscapeHeightmapTracker.ReverseMapping.VerticalTraceSegment", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapeSurfaceTraceHelperVerticalSegmentTest::RunTest(const FString& Parameters)
{
	const FVector WorldXY(100.0, -250.0, 75.0);
	const FLandscapeVerticalTraceSegment Segment = FLandscapeSurfaceTraceHelper::BuildVerticalTraceSegment(WorldXY, 500.0);

	TestTrue(TEXT("Segment is valid"), Segment.bIsValid);
	TestEqual(TEXT("Start X unchanged"), Segment.Start.X, WorldXY.X);
	TestEqual(TEXT("Start Y unchanged"), Segment.Start.Y, WorldXY.Y);
	TestEqual(TEXT("End X unchanged"), Segment.End.X, WorldXY.X);
	TestEqual(TEXT("End Y unchanged"), Segment.End.Y, WorldXY.Y);
	TestEqual(TEXT("Start above"), Segment.Start.Z, 575.0);
	TestEqual(TEXT("End below"), Segment.End.Z, -425.0);
	TestTrue(TEXT("Start Z above end Z"), Segment.Start.Z > Segment.End.Z);

	const FLandscapeVerticalTraceSegment Invalid = FLandscapeSurfaceTraceHelper::BuildVerticalTraceSegment(WorldXY, 0.0);
	TestFalse(TEXT("Zero trace distance rejected"), Invalid.bIsValid);
	TestFalse(TEXT("Invalid segment reports reason"), Invalid.FailureReason.IsEmpty());

	return true;
}

#endif
