#include "LandscapeHeightmapTrackerEdMode.h"

#include "Components/PrimitiveComponent.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "PrimitiveDrawInterface.h"
#include "ViewportTraceRayBuilder.h"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeHeightmapTrackerEdMode, Log, All);

bool FLandscapeHeightmapTrackerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed)
	{
		if (FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled())
		{
			TraceLandscapeClick(ViewportClient, Viewport);
		}
		return false;
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

void FLandscapeHeightmapTrackerEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	if (FLandscapeHeightmapTrackerModule::ConsumeReverseMarkerCleanupRequest())
	{
		FLandscapeHeightmapTrackerModule::ClearReverseMarker();
	}
}

void FLandscapeHeightmapTrackerEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	if (!PDI)
	{
		return;
	}

	FVector MarkerBase;
	if (!FLandscapeHeightmapTrackerModule::GetReverseMarker(MarkerBase))
	{
		return;
	}

	constexpr double MarkerHeight = 10000.0;
	const FVector MarkerTop = MarkerBase + FVector(0.0, 0.0, MarkerHeight);
	PDI->DrawLine(MarkerBase, MarkerTop, FLinearColor::Yellow, SDPG_Foreground, 4.0f, 0.0f, true);
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
	const FViewportTraceSegment TraceSegment = FViewportTraceRayBuilder::BuildTraceSegment(
		ViewportClient->IsPerspective(),
		Cursor.GetOrigin(),
		Cursor.GetDirection());
	if (!TraceSegment.bIsValid)
	{
		UE_LOG(
			LogLandscapeHeightmapTrackerEdMode,
			Verbose,
			TEXT("Viewport click trace skipped. ViewportType=%d IsPerspective=%s Mouse=(%d,%d) Origin=%s Direction=%s Reason=%s"),
			static_cast<int32>(ViewportClient->GetViewportType()),
			ViewportClient->IsPerspective() ? TEXT("true") : TEXT("false"),
			Viewport->GetMouseX(),
			Viewport->GetMouseY(),
			*Cursor.GetOrigin().ToCompactString(),
			*Cursor.GetDirection().ToCompactString(),
			*TraceSegment.FailureReason);
		return false;
	}

	UE_LOG(
		LogLandscapeHeightmapTrackerEdMode,
		Verbose,
		TEXT("Viewport click trace. ViewportType=%d IsPerspective=%s Mouse=(%d,%d) Origin=%s Direction=%s Start=%s End=%s"),
		static_cast<int32>(ViewportClient->GetViewportType()),
		ViewportClient->IsPerspective() ? TEXT("true") : TEXT("false"),
		Viewport->GetMouseX(),
		Viewport->GetMouseY(),
		*TraceSegment.Origin.ToCompactString(),
		*TraceSegment.Direction.ToCompactString(),
		*TraceSegment.Start.ToCompactString(),
		*TraceSegment.End.ToCompactString());

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandscapeHeightmapTrackerClick), true);
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.bTraceComplex = true;

	if (!World->LineTraceSingleByChannel(Hit, TraceSegment.Start, TraceSegment.End, ECC_Visibility, QueryParams))
	{
		return false;
	}

	UE_LOG(
		LogLandscapeHeightmapTrackerEdMode,
		Verbose,
		TEXT("Viewport click hit. Actor=%s Component=%s Impact=%s"),
		Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"),
		Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("None"),
		*Hit.ImpactPoint.ToCompactString());

	FLandscapeHeightmapTrackerModule::FViewportClickResult Result;
	Result.WorldPosition = Hit.ImpactPoint;
	Result.HitActor = Hit.GetActor();
	Result.HitComponent = Hit.GetComponent();
	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
	return true;
}
