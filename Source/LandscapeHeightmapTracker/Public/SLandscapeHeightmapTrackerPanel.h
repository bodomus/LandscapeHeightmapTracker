#pragma once

#include "CoreMinimal.h"
#include "LandscapeCoordinateMapper.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "Widgets/SCompoundWidget.h"

class ALandscapeProxy;
class UTexture2D;

class SLandscapeHeightmapTrackerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapeHeightmapTrackerPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SLandscapeHeightmapTrackerPanel() override;

private:
	FReply UseSelectedLandscape();
	FReply LoadHeightmap();
	FReply ClearMarker();
	void OnObjectSelected(const FAssetData& AssetData);
	void OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click);
	void OnViewportHover(const FLandscapeHeightmapTrackerModule::FViewportHoverResult& Hover);
	void OnHeightmapClicked(FVector2D DisplayUV);
	void SetTrackingEnabled(ECheckBoxState NewState);
	void SetFlipX(ECheckBoxState NewState);
	void SetFlipY(ECheckBoxState NewState);
	void AssignLandscape(ALandscapeProxy* InLandscape);
	void RefreshLandscapeBounds();
	void ReleaseTexture();
	bool LoadPngTexture(const FString& FilePath, FString& OutError);
	bool IsAssignedLandscapeHit(AActor* HitActor, UPrimitiveComponent* HitComponent) const;
	void ClearHoverMarker();
	bool SetHoverMarkerUV(const FVector2D& NewUV);
	void InvalidateHeightmapMarkerPaint();
	void UpdateStatus(const FText& InStatus);

	ECheckBoxState IsTrackingChecked() const;
	ECheckBoxState IsFlipXChecked() const;
	ECheckBoxState IsFlipYChecked() const;
	FText GetLandscapeNameText() const;
	FText GetActorLocationText() const;
	FText GetActorRotationText() const;
	FText GetActorScaleText() const;
	FText GetLocalBoundsText() const;
	FText GetImagePathText() const;
	FText GetImageInfoText() const;
	FText GetWorldText() const;
	FText GetLocalText() const;
	FText GetUvText() const;
	FText GetPixelText() const;
	FText GetStatusText() const;

	TWeakObjectPtr<ALandscapeProxy> AssignedLandscape;
	FLandscapeTrackerBounds LocalBounds;
	FLandscapeTrackerMappingResult LastMapping;
	FVector2D MarkerUV = FVector2D::ZeroVector;
	FVector2D HoverMarkerUV = FVector2D::ZeroVector;
	FVector2D LastLandscapeUV = FVector2D::ZeroVector;
	FIntPoint ImageSize = FIntPoint::ZeroValue;
	FString ImagePath;
	FString ImageFormat;
	FText StatusText;
	bool bHasMarker = false;
	bool bHasHoverMarker = false;
	bool bHoverTrackingHasViewportState = false;
	bool bHasLandscapeUV = false;
	bool bTrackClicks = false;
	bool bFlipX = false;
	bool bFlipY = true;

	UTexture2D* HeightmapTexture = nullptr;
	FSlateBrush HeightmapBrush;
	TSharedPtr<SWidget> HeightmapImageWidget;
	FDelegateHandle ClickDelegateHandle;
	FDelegateHandle HoverDelegateHandle;
};
