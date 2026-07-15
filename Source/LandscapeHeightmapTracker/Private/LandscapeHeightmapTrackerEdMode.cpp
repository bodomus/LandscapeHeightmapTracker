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

	if (!FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled())
	{
		ClearRememberedHoverViewport();
		return;
	}

	if (!IsRememberedHoverViewportRelevant(ViewportClient))
	{
		return;
	}

	FHoverViewState CurrentViewState;
	if (!HasRememberedViewChanged(ViewportClient, CurrentViewState))
	{
		return;
	}

	FViewportTraceResult TraceResult;
	TraceViewportUnderCursor(ViewportClient, LastHoverViewport, TraceResult);
	BroadcastHoverResult(TraceResult);
	LastHoverViewState = CurrentViewState;
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

bool FLandscapeHeightmapTrackerEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	const bool bBaseHandled = FEdMode::MouseMove(ViewportClient, Viewport, x, y);

	if (!FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled())
	{
		if (bHasRememberedHoverViewport)
		{
			ClearRememberedHoverViewport();
			BroadcastHoverClear();
		}
		return bBaseHandled;
	}

	RememberHoverViewport(ViewportClient, Viewport, x, y);

	FViewportTraceResult TraceResult;
	TraceViewportUnderCursor(ViewportClient, Viewport, TraceResult);
	BroadcastHoverResult(TraceResult);

	return bBaseHandled;
}

bool FLandscapeHeightmapTrackerEdMode::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	const bool bBaseHandled = FEdMode::MouseLeave(ViewportClient, Viewport);

	if (!bHasRememberedHoverViewport || Viewport == LastHoverViewport)
	{
		ClearRememberedHoverViewport(Viewport);
		if (FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled())
		{
			BroadcastHoverClear();
		}
	}

	return bBaseHandled;
}

bool FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const
{
	FViewportTraceResult TraceResult;
	if (!TraceViewportUnderCursor(ViewportClient, Viewport, TraceResult))
	{
		return false;
	}

	UE_LOG(
		LogLandscapeHeightmapTrackerEdMode,
		Verbose,
		TEXT("Viewport click hit. Actor=%s Component=%s Impact=%s"),
		TraceResult.HitActor.IsValid() ? *TraceResult.HitActor->GetName() : TEXT("None"),
		TraceResult.HitComponent.IsValid() ? *TraceResult.HitComponent->GetName() : TEXT("None"),
		*TraceResult.WorldPosition.ToCompactString());

	FLandscapeHeightmapTrackerModule::FViewportClickResult Result;
	Result.WorldPosition = TraceResult.WorldPosition;
	Result.HitActor = TraceResult.HitActor;
	Result.HitComponent = TraceResult.HitComponent;
	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
	return true;
}

bool FLandscapeHeightmapTrackerEdMode::TraceViewportUnderCursor(FEditorViewportClient* ViewportClient, FViewport* Viewport, FViewportTraceResult& OutResult) const
{
	if (!ViewportClient || !Viewport || !GEditor)
	{
		OutResult = FViewportTraceResult();
		return false;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		OutResult = FViewportTraceResult();
		return false;
	}

	const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
	const FViewportTraceSegment TraceSegment = FViewportTraceRayBuilder::BuildTraceSegment(
		ViewportClient->IsPerspective(),
		Cursor.GetOrigin(),
		Cursor.GetDirection());
	if (!TraceSegment.bIsValid)
	{
		OutResult = FViewportTraceResult();
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandscapeHeightmapTrackerClick), true);
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.bTraceComplex = true;

	if (!World->LineTraceSingleByChannel(Hit, TraceSegment.Start, TraceSegment.End, ECC_Visibility, QueryParams))
	{
		OutResult = FViewportTraceResult();
		return false;
	}

	OutResult.bHit = true;
	OutResult.WorldPosition = Hit.ImpactPoint;
	OutResult.HitActor = Hit.GetActor();
	OutResult.HitComponent = Hit.GetComponent();
	return true;
}

void FLandscapeHeightmapTrackerEdMode::BroadcastHoverResult(const FViewportTraceResult& TraceResult) const
{
	FLandscapeHeightmapTrackerModule::FViewportHoverResult Result;
	Result.bHasHit = TraceResult.bHit;
	Result.WorldPosition = TraceResult.WorldPosition;
	Result.HitActor = TraceResult.HitActor;
	Result.HitComponent = TraceResult.HitComponent;
	FLandscapeHeightmapTrackerModule::OnViewportHoverResult().Broadcast(Result);
}

void FLandscapeHeightmapTrackerEdMode::BroadcastHoverClear() const
{
	FLandscapeHeightmapTrackerModule::OnViewportHoverResult().Broadcast(FLandscapeHeightmapTrackerModule::FViewportHoverResult());
}

void FLandscapeHeightmapTrackerEdMode::RememberHoverViewport(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	LastHoverViewportClient = ViewportClient;
	LastHoverViewport = Viewport;
	LastHoverMousePosition = FIntPoint(MouseX, MouseY);
	LastHoverViewState = ViewportClient ? MakeHoverViewState(*ViewportClient) : FHoverViewState();
	bHasRememberedHoverViewport = ViewportClient && Viewport && IsMousePositionInsideViewport(Viewport, LastHoverMousePosition);
}

void FLandscapeHeightmapTrackerEdMode::ClearRememberedHoverViewport(FViewport* Viewport)
{
	if (Viewport && LastHoverViewport != Viewport)
	{
		return;
	}

	LastHoverViewportClient = nullptr;
	LastHoverViewport = nullptr;
	LastHoverMousePosition = FIntPoint::ZeroValue;
	LastHoverViewState = FHoverViewState();
	bHasRememberedHoverViewport = false;
}

bool FLandscapeHeightmapTrackerEdMode::IsRememberedHoverViewportRelevant(FEditorViewportClient* ViewportClient) const
{
	if (!bHasRememberedHoverViewport || !ViewportClient || ViewportClient != LastHoverViewportClient || !LastHoverViewport)
	{
		return false;
	}

	if (ViewportClient->Viewport != LastHoverViewport)
	{
		return false;
	}

	const FIntPoint CurrentMousePosition(LastHoverViewport->GetMouseX(), LastHoverViewport->GetMouseY());
	return IsMousePositionInsideViewport(LastHoverViewport, CurrentMousePosition);
}

bool FLandscapeHeightmapTrackerEdMode::HasRememberedViewChanged(FEditorViewportClient* ViewportClient, FHoverViewState& OutCurrentState) const
{
	if (!ViewportClient)
	{
		OutCurrentState = FHoverViewState();
		return false;
	}

	OutCurrentState = MakeHoverViewState(*ViewportClient);
	return !LastHoverViewState.Equals(OutCurrentState);
}

FLandscapeHeightmapTrackerEdMode::FHoverViewState FLandscapeHeightmapTrackerEdMode::MakeHoverViewState(const FEditorViewportClient& ViewportClient)
{
	FHoverViewState State;
	State.Location = ViewportClient.GetViewLocation();
	State.Rotation = ViewportClient.GetViewRotation();
	State.OrthoZoom = ViewportClient.GetOrthoZoom();
	return State;
}

bool FLandscapeHeightmapTrackerEdMode::IsMousePositionInsideViewport(FViewport* Viewport, const FIntPoint& MousePosition)
{
	if (!Viewport)
	{
		return false;
	}

	const FIntPoint ViewportSize = Viewport->GetSizeXY();
	return MousePosition.X >= 0
		&& MousePosition.Y >= 0
		&& MousePosition.X < ViewportSize.X
		&& MousePosition.Y < ViewportSize.Y;
}

bool FLandscapeHeightmapTrackerEdMode::FHoverViewState::Equals(const FHoverViewState& Other) const
{
	return Location.Equals(Other.Location, KINDA_SMALL_NUMBER)
		&& Rotation.Equals(Other.Rotation, KINDA_SMALL_NUMBER)
		&& FMath::IsNearlyEqual(OrthoZoom, Other.OrthoZoom, KINDA_SMALL_NUMBER);
}
