#include "LandscapePaintLayerService.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLandscapePaintLayerValidationTest,
	"LandscapeHeightmapTracker.PaintLayers.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapePaintLayerValidationTest::RunTest(const FString& Parameters)
{
	FLandscapePaintTargetLayerInfo Grass;
	Grass.LayerName = TEXT("Grass");
	FLandscapePaintTargetLayerInfo Rock;
	Rock.LayerName = TEXT("Rock");
	const TArray<FLandscapePaintTargetLayerInfo> Available = {Grass, Rock};

	TArray<FName> Validated;
	FString Error;
	TestFalse(TEXT("Empty request rejected"), FLandscapePaintLayerService::ValidateLayerNames(Available, {}, Validated, Error));
	TestTrue(TEXT("Single known layer accepted"), FLandscapePaintLayerService::ValidateLayerNames(Available, {TEXT("Grass")}, Validated, Error));
	TestEqual(TEXT("Accepted name preserved"), Validated, TArray<FName>({TEXT("Grass")}));
	TestTrue(TEXT("Multiple known layers accepted"), FLandscapePaintLayerService::ValidateLayerNames(Available, {TEXT("Grass"), TEXT("Rock")}, Validated, Error));
	TestFalse(TEXT("Unknown layer rejected"), FLandscapePaintLayerService::ValidateLayerNames(Available, {TEXT("Mud")}, Validated, Error));
	TestFalse(TEXT("Duplicate layer rejected"), FLandscapePaintLayerService::ValidateLayerNames(Available, {TEXT("Grass"), TEXT("Grass")}, Validated, Error));
	TestFalse(TEXT("Empty name rejected"), FLandscapePaintLayerService::ValidateLayerNames(Available, {NAME_None}, Validated, Error));
	return true;
}
