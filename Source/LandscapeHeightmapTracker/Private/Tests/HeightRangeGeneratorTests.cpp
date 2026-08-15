#include "HeightRangeGenerator.h"

#include "Misc/AutomationTest.h"

namespace
{
const FLinearColor Cyan(0.0f, 0.8f, 1.0f, 1.0f);

FHeightRangeDefinition MakeRange(double Min, double Max, const FLinearColor& Color, bool bEnabled = true)
{
	FHeightRangeDefinition Range;
	Range.MinHeightMeters = Min;
	Range.MaxHeightMeters = Max;
	Range.Color = Color;
	Range.bEnabled = bEnabled;
	return Range;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeightRangeNormalizeAndSortTest,
	"LandscapeHeightmapTracker.HeightRanges.NormalizeAndSort",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightRangeNormalizeAndSortTest::RunTest(const FString& Parameters)
{
	TArray<FHeightRangeDefinition> Ranges =
	{
		MakeRange(40.0, 20.0, FLinearColor::Green),
		MakeRange(10.0, 0.0, Cyan)
	};
	FHeightRangeGenerator::NormalizeAndSort(Ranges);
	TestEqual(TEXT("Lower range sorts first"), Ranges[0].MinHeightMeters, 0.0);
	TestEqual(TEXT("Reversed lower range is normalized"), Ranges[0].MaxHeightMeters, 10.0);
	TestEqual(TEXT("Upper range minimum is normalized"), Ranges[1].MinHeightMeters, 20.0);
	TestEqual(TEXT("Upper range maximum is normalized"), Ranges[1].MaxHeightMeters, 40.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeightRangeConflictTest,
	"LandscapeHeightmapTracker.HeightRanges.ConflictValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightRangeConflictTest::RunTest(const FString& Parameters)
{
	const TArray<FHeightRangeDefinition> Existing =
	{
		MakeRange(0.0, 20.0, Cyan)
	};
	TestEqual(
		TEXT("Exact duplicate is rejected first"),
		FHeightRangeGenerator::FindConflict(MakeRange(20.0, 0.0, FLinearColor::Red), Existing).Conflict,
		EHeightRangeConflict::Duplicate);
	TestEqual(
		TEXT("Strict interior overlap is rejected"),
		FHeightRangeGenerator::FindConflict(MakeRange(10.0, 30.0, FLinearColor::Red), Existing).Conflict,
		EHeightRangeConflict::Overlap);
	TestEqual(
		TEXT("Touching boundary is allowed"),
		FHeightRangeGenerator::FindConflict(MakeRange(20.0, 40.0, FLinearColor::Green), Existing).Conflict,
		EHeightRangeConflict::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeightRangeBoundaryOwnershipTest,
	"LandscapeHeightmapTracker.HeightRanges.BoundaryOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightRangeBoundaryOwnershipTest::RunTest(const FString& Parameters)
{
	const TArray<float> Heights =
	{
		0.0f, 10.0f, 20.0f, 30.0f, 40.0f,
		0.0f, 10.0f, 20.0f, 30.0f, 40.0f
	};
	const TArray<FHeightRangeDefinition> Ranges =
	{
		MakeRange(0.0, 20.0, Cyan),
		MakeRange(20.0, 40.0, FLinearColor::Green)
	};
	const FMultiHeightRangeResult Result = FHeightRangeGenerator::Generate(Heights, 5, 2, Ranges);

	TestEqual(TEXT("Every pixel has one ownership entry"), Result.RangeIndexByPixel.Num(), 10);
	TestEqual(TEXT("Lower [Min, Max) range owns 0 and 10"), Result.PixelCounts[0], 4);
	TestEqual(TEXT("Upper range owns shared boundary and inclusive global maximum"), Result.PixelCounts[1], 6);
	TestEqual(TEXT("Shared boundary belongs to upper range"), Result.RangeIndexByPixel[2], 1);
	TestEqual(TEXT("Global maximum belongs to uppermost range"), Result.RangeIndexByPixel[4], 1);

	int32 SharedBoundaryContourCount = 0;
	for (const FHeightContour& Contour : Result.Contours)
	{
		if (FMath::IsNearlyEqual(Contour.BoundaryHeightMeters, 20.0))
		{
			++SharedBoundaryContourCount;
			TestTrue(TEXT("Shared contour uses upper range color"), Contour.Color.Equals(FLinearColor::Green));
		}
	}
	TestEqual(TEXT("Shared boundary contour is generated once"), SharedBoundaryContourCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHeightRangeDisabledTest,
	"LandscapeHeightmapTracker.HeightRanges.DisabledRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightRangeDisabledTest::RunTest(const FString& Parameters)
{
	const TArray<float> Heights =
	{
		0.0f, 10.0f, 20.0f, 30.0f, 40.0f,
		0.0f, 10.0f, 20.0f, 30.0f, 40.0f
	};
	const TArray<FHeightRangeDefinition> Ranges =
	{
		MakeRange(0.0, 20.0, Cyan),
		MakeRange(20.0, 40.0, FLinearColor::Green, false)
	};
	const FMultiHeightRangeResult Result = FHeightRangeGenerator::Generate(Heights, 5, 2, Ranges);
	TestEqual(TEXT("Upper disabled range has no pixels"), Result.PixelCounts[1], 0);
	TestEqual(TEXT("Uppermost enabled maximum remains inclusive"), Result.PixelCounts[0], 6);
	TestEqual(TEXT("Disabled range interior remains unowned"), Result.RangeIndexByPixel[3], INDEX_NONE);
	return true;
}
