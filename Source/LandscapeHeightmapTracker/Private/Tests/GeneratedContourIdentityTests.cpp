#include "ContourBoxesEditorService.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Editor.h"
#include "HeightZoneGenerator.h"
#include "Landscape.h"
#include "LHTContourBoxesActor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGeneratedContourIdentityTest,
	"LandscapeHeightmapTracker.ContourBoxes.GeneratedTopologyIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGeneratedContourIdentityTest::RunTest(const FString& Parameters)
{
	TArray<float> Heights;
	Heights.SetNumZeroed(27);
	Heights[3 + 9] = 10.0f;
	Heights[7 + 9] = 10.0f;
	const auto Generate = [&Heights]()
	{
		return FHeightZoneGenerator::Generate(Heights, 9, 3, 5.0, EHeightZoneMode::ContourOnly);
	};
	const FHeightZoneResult Original = Generate();
	if (!TestEqual(TEXT("Two generated same-height contours"), Original.Contours.Num(), 2)) { return false; }
	TestNotEqual(TEXT("Separate components have separate IDs"), Original.Contours[0].Id, Original.Contours[1].Id);
	const FHeightZoneResult Repeated = Generate();
	if (!TestEqual(TEXT("Repeated generation preserves count"), Repeated.Contours.Num(), 2)) { return false; }
	TestEqual(TEXT("Unchanged generation preserves first ID"), Repeated.Contours[0].Id, Original.Contours[0].Id);
	TestEqual(TEXT("Unchanged generation preserves second ID"), Repeated.Contours[1].Id, Original.Contours[1].Id);

	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Editor world"), World)) { return false; }
	ALandscape* Landscape = World->SpawnActor<ALandscape>();
	if (!TestNotNull(TEXT("Source Landscape"), Landscape)) { return false; }
	FLandscapeTrackerBounds Bounds;
	Bounds.Min = FVector2D::ZeroVector;
	const FVector Scale = Landscape->GetActorScale3D();
	Bounds.Max = FVector2D(8000.0 / Scale.X, 2000.0 / Scale.Y);
	FLandscapeTrackerMappingOptions MappingOptions;
	MappingOptions.bFlipY = false;
	FContourBoxSettings Settings;
	FString Error;
	ALHTContourBoxesActor* FirstActor = nullptr;
	ALHTContourBoxesActor* SecondActor = nullptr;
	if (!TestTrue(TEXT("Create first generated set"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, Original.Contours[0], Settings, false, FirstActor, Error)) ||
		!TestTrue(TEXT("Create second generated set"), FContourBoxesEditorService::CreateOrUpdate(
			Landscape, Bounds, MappingOptions, Original.Contours[1], Settings, false, SecondActor, Error)))
	{
		AddError(Error);
		return false;
	}
	UHierarchicalInstancedStaticMeshComponent* FirstHISM = FirstActor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>();
	if (!TestNotNull(TEXT("Generated actor retains HISM"), FirstHISM)) { return false; }
	TArray<FTransform> FirstTransforms;
	for (int32 Index = 0; Index < FirstActor->GetInstanceCount(); ++Index)
	{
		FTransform Transform;
		FirstHISM->GetInstanceTransform(Index, Transform, true);
		FirstTransforms.Add(Transform);
	}
	const auto FirstUnchanged = [&]()
	{
		return FContourBoxesEditorService::FindActor(Landscape, Original.Contours[0].Id) == FirstActor &&
			FirstActor->IsEquivalent(Landscape, Original.Contours[0].Id, 5.0, Original.Contours[0].Color, FirstTransforms);
	};
	const int32 OriginalSecondCount = SecondActor->GetInstanceCount();

	// This is the production generator path, rather than manually assigned GUIDs.
	Heights[3 + 9] = 0.0f;
	const FHeightZoneResult Removed = Generate();
	if (!TestEqual(TEXT("First contour disappears"), Removed.Contours.Num(), 1)) { return false; }
	const FHeightContour& Survivor = Removed.Contours[0];
	TestEqual(TEXT("Survivor keeps its own ID after index shifts"), Survivor.Id, Original.Contours[1].Id);
	TestNotEqual(TEXT("Survivor never adopts disappeared contour ID"), Survivor.Id, Original.Contours[0].Id);
	Settings.BoxLengthMeters = 5.0;
	Settings.GapLengthMeters = 5.0;
	ALHTContourBoxesActor* UpdatedActor = nullptr;
	if (!TestTrue(TEXT("Update survivor"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, Survivor, Settings, true, UpdatedActor, Error))) { AddError(Error); return false; }
	TestEqual(TEXT("Update targets second actor"), UpdatedActor, SecondActor);
	TestTrue(TEXT("Update changes survivor instances"), SecondActor->GetInstanceCount() > OriginalSecondCount);
	TestTrue(TEXT("Update preserves disappeared contour actor and transforms"), FirstUnchanged());
	const int32 UpdatedSecondCount = SecondActor->GetInstanceCount();
	GEditor->UndoTransaction();
	SecondActor = FContourBoxesEditorService::FindActor(Landscape, Survivor.Id);
	if (!TestNotNull(TEXT("Undo keeps survivor actor"), SecondActor)) { return false; }
	TestEqual(TEXT("Undo restores survivor instances"), SecondActor->GetInstanceCount(), OriginalSecondCount);
	TestTrue(TEXT("Undo update preserves first set"), FirstUnchanged());
	GEditor->RedoTransaction();
	SecondActor = FContourBoxesEditorService::FindActor(Landscape, Survivor.Id);
	if (!TestNotNull(TEXT("Redo keeps survivor actor"), SecondActor)) { return false; }
	TestEqual(TEXT("Redo restores updated survivor instances"), SecondActor->GetInstanceCount(), UpdatedSecondCount);

	bool bChanged = false;
	TestTrue(TEXT("Delete survivor"), FContourBoxesEditorService::Delete(Landscape, Survivor.Id, bChanged, Error));
	TestTrue(TEXT("Survivor deletion changes level"), bChanged);
	TestNull(TEXT("Only survivor actor is removed"), FContourBoxesEditorService::FindActor(Landscape, Survivor.Id));
	TestTrue(TEXT("Delete preserves disappeared contour actor and transforms"), FirstUnchanged());
	GEditor->UndoTransaction();
	TestNotNull(TEXT("Undo restores survivor set"), FContourBoxesEditorService::FindActor(Landscape, Survivor.Id));
	TestTrue(TEXT("Undo delete preserves first set"), FirstUnchanged());
	GEditor->RedoTransaction();
	TestNull(TEXT("Redo deletes survivor only"), FContourBoxesEditorService::FindActor(Landscape, Survivor.Id));
	TestTrue(TEXT("Redo delete preserves first set"), FirstUnchanged());

	if (!TestTrue(TEXT("Recreate survivor"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, Survivor, Settings, false, SecondActor, Error))) { AddError(Error); return false; }
	Heights[1 + 9] = 10.0f;
	const FHeightZoneResult Added = Generate();
	if (!TestEqual(TEXT("An earlier component is added"), Added.Contours.Num(), 2)) { return false; }
	TestEqual(TEXT("Survivor ID is also stable across insertion"), Added.Contours[1].Id, Survivor.Id);
	TestNotEqual(TEXT("New component cannot adopt disappeared ID"), Added.Contours[0].Id, Original.Contours[0].Id);

	// Changed geometry is ambiguous: Update/Delete cannot adopt either old set.
	Heights[7 + 9] = 20.0f;
	const FHeightZoneResult Changed = Generate();
	if (!TestEqual(TEXT("Changed geometry still has two contours"), Changed.Contours.Num(), 2)) { return false; }
	const FHeightContour& ChangedContour = Changed.Contours[1];
	TestNotEqual(TEXT("Changed geometry gets a new identity"), ChangedContour.Id, Survivor.Id);
	TestFalse(TEXT("Changed contour cannot update old actor"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, ChangedContour, Settings, true, UpdatedActor, Error));
	TestNull(TEXT("Rejected update does not return an old actor"), UpdatedActor);
	TestTrue(TEXT("Delete unknown changed contour is safe"), FContourBoxesEditorService::Delete(Landscape, ChangedContour.Id, bChanged, Error));
	TestFalse(TEXT("Unknown contour deletion changes nothing"), bChanged);
	TestEqual(TEXT("Old survivor actor is preserved"), FContourBoxesEditorService::FindActor(Landscape, Survivor.Id), SecondActor);
	TestEqual(TEXT("Old survivor instance count is preserved"), SecondActor->GetInstanceCount(), UpdatedSecondCount);
	TestTrue(TEXT("Ambiguous operations preserve first set"), FirstUnchanged());
	ALHTContourBoxesActor* NewActor = nullptr;
	if (!TestTrue(TEXT("Explicit Create makes a new set for changed geometry"), FContourBoxesEditorService::CreateOrUpdate(
		Landscape, Bounds, MappingOptions, ChangedContour, Settings, false, NewActor, Error))) { AddError(Error); return false; }
	TestNotEqual(TEXT("Changed geometry does not reuse original survivor"), NewActor, SecondActor);
	TestTrue(TEXT("Delete new geometry set"), FContourBoxesEditorService::Delete(Landscape, ChangedContour.Id, bChanged, Error));
	TestEqual(TEXT("Deleting new set leaves old survivor actor"), FContourBoxesEditorService::FindActor(Landscape, Survivor.Id), SecondActor);
	TestTrue(TEXT("Deleting new set leaves first set intact"), FirstUnchanged());
	return true;
}

#endif
