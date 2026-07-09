#if WITH_DEV_AUTOMATION_TESTS

#include "HeightmapImageClickMapper.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapImageClickMapperFittedRectTest, "LandscapeHeightmapTracker.ReverseMapping.FittedImageClick", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapImageClickMapperFittedRectTest::RunTest(const FString& Parameters)
{
	const FHeightmapFittedImageRect Rect = FHeightmapImageClickMapper::CalculateFittedImageRect(FVector2D(1000.0, 500.0), FVector2D(1000.0, 1000.0));
	TestTrue(TEXT("Fitted rect is valid"), Rect.bIsValid);
	TestEqual(TEXT("Draw size"), Rect.DrawSize, FVector2D(500.0, 500.0));
	TestEqual(TEXT("Draw offset"), Rect.DrawOffset, FVector2D(250.0, 0.0));

	const FHeightmapClickMappingResult TopLeft = FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(FVector2D(250.0, 0.0), Rect);
	TestTrue(TEXT("Top left valid"), TopLeft.bIsValid);
	TestEqual(TEXT("Top left UV"), TopLeft.DisplayUV, FVector2D(0.0, 0.0));

	const FHeightmapClickMappingResult Center = FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(FVector2D(500.0, 250.0), Rect);
	TestTrue(TEXT("Center valid"), Center.bIsValid);
	TestEqual(TEXT("Center UV"), Center.DisplayUV, FVector2D(0.5, 0.5));

	const FHeightmapClickMappingResult BottomRight = FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(FVector2D(750.0, 500.0), Rect);
	TestTrue(TEXT("Bottom right valid"), BottomRight.bIsValid);
	TestEqual(TEXT("Bottom right UV"), BottomRight.DisplayUV, FVector2D(1.0, 1.0));

	TestFalse(TEXT("Left letterbox rejected"), FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(FVector2D(100.0, 250.0), Rect).bIsValid);
	TestFalse(TEXT("Right letterbox rejected"), FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(FVector2D(900.0, 250.0), Rect).bIsValid);

	return true;
}

#endif
