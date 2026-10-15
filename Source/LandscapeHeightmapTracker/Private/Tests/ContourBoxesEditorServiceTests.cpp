#include "ContourBoxesEditorService.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "LHTContourBoxesActor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
FHeightContour MakeContour(const FGuid& Id, double Y)
{
	FHeightContour Contour;
	Contour.Id = Id;
	Contour.BoundaryHeightMeters = 100.0;
	Contour.Color = FLinearColor::Green;
	Contour.Points = {FVector2D(0.0, Y), FVector2D(1.0, Y)};
	return Contour;
}

int32 CountManagedActors(UWorld* World)
{
	int32 Count = 0;
	for (TActorIterator<ALHTContourBoxesActor> It(World); It; ++It)
	{
		++Count;
	}
	return Count;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContourBoxesEditorLifecycleTest,
	"LandscapeHeightmapTracker.ContourBoxes.EditorLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContourBoxesEditorLifecycleTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Editor test world"), World))
	{
		return false;
	}
	ALandscape* Landscape = World->SpawnActor<ALandscape>();
	if (!TestNotNull(TEXT("Source Landscape"), Landscape))
	{
		return false;
	}

	FLandscapeTrackerBounds Bounds;
	Bounds.Min = FVector2D::ZeroVector;
	const FVector LandscapeScale = Landscape->GetActorScale3D();
	Bounds.Max = FVector2D(5000.0 / LandscapeScale.X, 5000.0 / LandscapeScale.Y);
	FLandscapeTrackerMappingOptions MappingOptions;
	MappingOptions.bFlipY = false;
	const FGuid FirstId = FGuid::NewGuid();
	const FGuid SecondId = FGuid::NewGuid();
	const FHeightContour FirstContour = MakeContour(FirstId, 0.0);
	const FHeightContour SecondContour = MakeContour(SecondId, 0.5);
	FContourBoxSettings Settings;
	FString Error;
	ALHTContourBoxesActor* FirstActor = nullptr;

	if (!TestTrue(TEXT("Create first actor"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, FirstContour, Settings, false, FirstActor, Error)))
	{
		AddError(Error);
		return false;
	}
	TestNotNull(TEXT("First actor returned"), FirstActor);
	TestEqual(TEXT("Default placement creates three instances"), FirstActor->GetInstanceCount(), 3);
	TestTrue(TEXT("Actor is editor only"), FirstActor->IsEditorOnly());
	TestFalse(TEXT("Actor collision disabled"), FirstActor->GetActorEnableCollision());
	TestEqual(TEXT("One generated actor"), CountManagedActors(World), 1);

	GEditor->UndoTransaction();
	TestNull(TEXT("Undo removes created actor"), FContourBoxesEditorService::FindActor(Landscape, FirstId));
	GEditor->RedoTransaction();
	FirstActor = FContourBoxesEditorService::FindActor(Landscape, FirstId);
	TestNotNull(TEXT("Redo restores created actor"), FirstActor);

	World->GetCurrentLevel()->GetPackage()->SetDirtyFlag(false);
	ALHTContourBoxesActor* ReusedActor = nullptr;
	Error.Reset();
	TestTrue(TEXT("Repeated Create succeeds as update"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, FirstContour, Settings, false, ReusedActor, Error));
	TestEqual(TEXT("Repeated Create reuses actor"), ReusedActor, FirstActor);
	TestEqual(TEXT("Repeated Create does not duplicate actor"), CountManagedActors(World), 1);
	TestFalse(TEXT("No-op repeated Create does not dirty the level"), World->GetCurrentLevel()->GetPackage()->IsDirty());

	ALHTContourBoxesActor* SecondActor = nullptr;
	Error.Reset();
	TestTrue(TEXT("Same-height second contour creates independently"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, SecondContour, Settings, false, SecondActor, Error));
	TestNotEqual(TEXT("Same-height contours use different actors"), FirstActor, SecondActor);
	TestEqual(TEXT("Two independent generated actors"), CountManagedActors(World), 2);

	Settings.BoxLengthMeters = 5.0;
	Settings.GapLengthMeters = 5.0;
	FirstActor->SetActorLabel(TEXT("User Preserved Label"), false);
	FirstActor->SetFolderPath(TEXT("User/Preserved/Folder"));
	Error.Reset();
	TestTrue(TEXT("Update existing actor"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, FirstContour, Settings, true, FirstActor, Error));
	TestEqual(TEXT("Update rebuilds instance count"), FirstActor->GetInstanceCount(), 5);
	TestEqual(TEXT("Update leaves the other contour unchanged"), SecondActor->GetInstanceCount(), 3);
	TestEqual(TEXT("Update preserves actor label"), FirstActor->GetActorLabel(), FString(TEXT("User Preserved Label")));
	TestEqual(TEXT("Update preserves Outliner folder"), FirstActor->GetFolderPath(), FName(TEXT("User/Preserved/Folder")));

	bool bDeleted = false;
	Error.Reset();
	TestTrue(TEXT("Delete first contour actor"), FContourBoxesEditorService::Delete(Landscape, FirstId, bDeleted, Error));
	TestTrue(TEXT("Delete reports a change"), bDeleted);
	TestNull(TEXT("Only first actor is deleted"), FContourBoxesEditorService::FindActor(Landscape, FirstId));
	TestNotNull(TEXT("Second actor remains"), FContourBoxesEditorService::FindActor(Landscape, SecondId));

	GEditor->UndoTransaction();
	TestNotNull(TEXT("Undo restores deleted actor"), FContourBoxesEditorService::FindActor(Landscape, FirstId));
	GEditor->RedoTransaction();
	TestNull(TEXT("Redo deletes actor again"), FContourBoxesEditorService::FindActor(Landscape, FirstId));
	TestNotNull(TEXT("Redo still preserves other contour"), FContourBoxesEditorService::FindActor(Landscape, SecondId));

	bDeleted = true;
	Error.Reset();
	TestTrue(TEXT("Deleting an already missing actor is safe"), FContourBoxesEditorService::Delete(Landscape, FirstId, bDeleted, Error));
	TestFalse(TEXT("Missing delete reports no change"), bDeleted);
	return true;
}

#endif
