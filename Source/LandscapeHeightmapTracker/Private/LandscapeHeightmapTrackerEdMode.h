#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class AActor;
class UPrimitiveComponent;

class FLandscapeHeightmapTrackerEdMode : public FEdMode
{
public:
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;

private:
	struct FViewportTraceResult
	{
		bool bHit = false;
		FVector WorldPosition = FVector::ZeroVector;
		TWeakObjectPtr<AActor> HitActor;
		TWeakObjectPtr<UPrimitiveComponent> HitComponent;
	};

	struct FHoverViewState
	{
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float OrthoZoom = 0.0f;

		bool Equals(const FHoverViewState& Other) const;
	};

	bool TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const;
	bool TraceViewportUnderCursor(FEditorViewportClient* ViewportClient, FViewport* Viewport, FViewportTraceResult& OutResult) const;
	void BroadcastHoverResult(const FViewportTraceResult& TraceResult) const;
	void BroadcastHoverClear() const;
	void RememberHoverViewport(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY);
	void ClearRememberedHoverViewport(FViewport* Viewport = nullptr);
	bool IsRememberedHoverViewportRelevant(FEditorViewportClient* ViewportClient) const;
	bool HasRememberedViewChanged(FEditorViewportClient* ViewportClient, FHoverViewState& OutCurrentState) const;
	static FHoverViewState MakeHoverViewState(const FEditorViewportClient& ViewportClient);
	static bool IsMousePositionInsideViewport(FViewport* Viewport, const FIntPoint& MousePosition);

	FEditorViewportClient* LastHoverViewportClient = nullptr;
	FViewport* LastHoverViewport = nullptr;
	FIntPoint LastHoverMousePosition = FIntPoint::ZeroValue;
	FHoverViewState LastHoverViewState;
	bool bHasRememberedHoverViewport = false;
};
