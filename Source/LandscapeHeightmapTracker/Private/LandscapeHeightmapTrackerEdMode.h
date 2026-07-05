#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class FLandscapeHeightmapTrackerEdMode : public FEdMode
{
public:
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

private:
	bool TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const;
};
