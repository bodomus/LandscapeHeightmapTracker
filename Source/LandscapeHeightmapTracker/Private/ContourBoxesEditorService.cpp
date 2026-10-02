#include "ContourBoxesEditorService.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "LHTContourBoxesActor.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "ContourBoxesEditorService"

namespace
{
FString HeightToken(double HeightMeters)
{
	FString Token = FString::Printf(TEXT("%.3f"), HeightMeters);
	while (Token.Contains(TEXT(".")) && Token.EndsWith(TEXT("0")))
	{
		Token.LeftChopInline(1);
	}
	if (Token.EndsWith(TEXT(".")))
	{
		Token.LeftChopInline(1);
	}
	Token.ReplaceInline(TEXT("-"), TEXT("Minus_"));
	Token.ReplaceInline(TEXT("."), TEXT("_"));
	return Token;
}

FString ShortGuid(const FGuid& Guid)
{
	return Guid.ToString(EGuidFormats::Digits).Left(8);
}
}

ALHTContourBoxesActor* FContourBoxesEditorService::FindActor(
	const ALandscapeProxy* Landscape,
	const FGuid& ContourId)
{
	if (!IsValid(Landscape) || !ContourId.IsValid() || !Landscape->GetWorld())
	{
		return nullptr;
	}
	for (TActorIterator<ALHTContourBoxesActor> It(Landscape->GetWorld()); It; ++It)
	{
		if (IsValid(*It) && It->Matches(Landscape, ContourId))
		{
			return *It;
		}
	}
	return nullptr;
}

bool FContourBoxesEditorService::CreateOrUpdate(
	ALandscapeProxy* Landscape,
	const FLandscapeTrackerBounds& LocalBounds,
	const FLandscapeTrackerMappingOptions& MappingOptions,
	const FHeightContour& Contour,
	const FContourBoxSettings& Settings,
	bool bRequireExistingActor,
	ALHTContourBoxesActor*& OutActor,
	FString& OutError)
{
	OutActor = nullptr;
	if (!IsInGameThread())
	{
		OutError = TEXT("3D contour boxes can only be changed on the editor thread.");
		return false;
	}
	if (!IsValid(Landscape) || !Landscape->GetWorld() || Landscape->GetWorld()->IsGameWorld())
	{
		OutError = TEXT("The assigned Landscape is not in a valid editor world.");
		return false;
	}
	if (!Contour.Id.IsValid())
	{
		OutError = TEXT("The contour has no stable identifier.");
		return false;
	}

	TArray<FVector> WorldPoints;
	if (!BuildWorldPoints(Landscape, LocalBounds, MappingOptions, Contour, WorldPoints, OutError))
	{
		return false;
	}
	const FContourBoxPlacementResult Placement = FContourBoxPlacement::Generate(
		WorldPoints,
		Contour.bClosed,
		Contour.BoundaryHeightMeters,
		Settings);
	if (!Placement.bIsValid)
	{
		OutError = Placement.Error;
		return false;
	}
	if (Placement.Transforms.IsEmpty())
	{
		OutError = FString::Printf(
			TEXT("The contour is %.2f m long, shorter than one %.2f m box."),
			Placement.PolylineLengthMeters,
			Settings.BoxLengthMeters);
		return false;
	}

	UWorld* World = Landscape->GetWorld();
	ALHTContourBoxesActor* Actor = FindActor(Landscape, Contour.Id);
	if (bRequireExistingActor && !Actor)
	{
		OutError = TEXT("The generated actor no longer exists. Use Create 3D Boxes to create it again.");
		return false;
	}
	if (Actor && Actor->IsEquivalent(
		Landscape,
		Contour.Id,
		Contour.BoundaryHeightMeters,
		Contour.Color,
		Placement.Transforms))
	{
		OutActor = Actor;
		return true;
	}

	const FScopedTransaction Transaction(Actor
		? LOCTEXT("UpdateContourBoxes", "Update 3D Contour Boxes")
		: LOCTEXT("CreateContourBoxes", "Create 3D Contour Boxes"));
	const bool bCreatingActor = Actor == nullptr;
	if (bCreatingActor)
	{
		if (World->GetCurrentLevel())
		{
			World->GetCurrentLevel()->Modify();
		}
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.OverrideLevel = World->GetCurrentLevel();
		SpawnParameters.ObjectFlags |= RF_Transactional;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Actor = World->SpawnActor<ALHTContourBoxesActor>(
			ALHTContourBoxesActor::StaticClass(),
			FTransform::Identity,
			SpawnParameters);
		if (!Actor)
		{
			OutError = TEXT("Unreal Editor could not create the contour boxes actor.");
			return false;
		}
	}

	Actor->Modify();
	Actor->ConfigureIdentity(Landscape, Contour.Id, Contour.BoundaryHeightMeters, Contour.Color);
	Actor->RebuildInstances(Placement.Transforms);
	if (bCreatingActor)
	{
		const FString Height = HeightToken(Contour.BoundaryHeightMeters);
		Actor->SetActorLabel(FString::Printf(TEXT("LHT_Contour_%sm_%s"), *Height, *ShortGuid(Contour.Id)), false);
		Actor->SetFolderPath(FName(*FString::Printf(
			TEXT("LandscapeHeightmapTracker/GeneratedContours/Height_%sm"),
			*Height)));
	}
	Actor->MarkPackageDirty();
	if (Actor->GetLevel())
	{
		Actor->GetLevel()->MarkPackageDirty();
	}
	OutActor = Actor;
	return true;
}

bool FContourBoxesEditorService::Delete(
	ALandscapeProxy* Landscape,
	const FGuid& ContourId,
	bool& bOutChanged,
	FString& OutError)
{
	bOutChanged = false;
	if (!IsInGameThread())
	{
		OutError = TEXT("3D contour boxes can only be changed on the editor thread.");
		return false;
	}
	ALHTContourBoxesActor* Actor = FindActor(Landscape, ContourId);
	if (!Actor)
	{
		return true;
	}
	UWorld* World = Actor->GetWorld();
	if (!World || World->IsGameWorld())
	{
		OutError = TEXT("The generated actor is not in a valid editor world.");
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteContourBoxes", "Delete 3D Contour Boxes"));
	Actor->Modify();
	if (!World->EditorDestroyActor(Actor, true))
	{
		OutError = TEXT("Unreal Editor could not delete the contour boxes actor.");
		return false;
	}
	bOutChanged = true;
	return true;
}

bool FContourBoxesEditorService::BuildWorldPoints(
	const ALandscapeProxy* Landscape,
	const FLandscapeTrackerBounds& LocalBounds,
	const FLandscapeTrackerMappingOptions& MappingOptions,
	const FHeightContour& Contour,
	TArray<FVector>& OutWorldPoints,
	FString& OutError)
{
	if (!IsValid(Landscape) || !LocalBounds.IsValid())
	{
		OutError = TEXT("Landscape bounds are unavailable.");
		return false;
	}
	OutWorldPoints.Reserve(Contour.Points.Num());
	for (const FVector2D& DisplayUV : Contour.Points)
	{
		const FLandscapeTrackerReverseMappingResult Mapping =
			FLandscapeCoordinateMapper::MapUVToLocalPosition(LocalBounds, DisplayUV, MappingOptions);
		if (!Mapping.bIsValid)
		{
			OutError = Mapping.FailureReason;
			return false;
		}
		OutWorldPoints.Add(Landscape->GetActorTransform().TransformPosition(Mapping.LocalPosition));
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
