#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class FLandscapeHeightmapTrackerEdMode : public FEdMode
{
public:
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

private:
	bool TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const;
};
