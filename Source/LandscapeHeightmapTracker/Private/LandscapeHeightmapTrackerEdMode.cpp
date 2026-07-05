#include "LandscapeHeightmapTrackerEdMode.h"

#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "Components/PrimitiveComponent.h"

bool FLandscapeHeightmapTrackerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed)
	{
		TraceLandscapeClick(ViewportClient, Viewport);
		return false;
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

bool FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const
{
	if (!ViewportClient || !Viewport || !GEditor)
	{
		return false;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return false;
	}

	const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
	const FVector Start = Cursor.GetOrigin();
	const FVector End = Start + Cursor.GetDirection() * HALF_WORLD_MAX;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandscapeHeightmapTrackerClick), true);
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.bTraceComplex = true;

	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		return false;
	}

	FLandscapeHeightmapTrackerModule::FViewportClickResult Result;
	Result.WorldPosition = Hit.ImpactPoint;
	Result.HitActor = Hit.GetActor();
	Result.HitComponent = Hit.GetComponent();
	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
	return true;
}
