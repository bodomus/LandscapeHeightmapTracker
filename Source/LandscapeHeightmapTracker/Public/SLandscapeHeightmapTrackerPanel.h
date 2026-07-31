#pragma once

#include "CoreMinimal.h"
#include "HeightZoneTypes.h"
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
	FReply ApplyHeightZone();
	FReply ClearHeightZone();
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
	void ReleaseHeightZoneTexture();
	bool LoadPngTexture(const FString& FilePath, FString& OutError);
	bool EnsureHeightMetersCache(FString& OutError);
	bool UpdateHeightZoneTexture(FString& OutError);
	void InvalidateHeightMetersCache();
	void ClearHeightZoneVisualization();
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
	FText GetImageColorModelText() const;
	FText GetImageBitDepthText() const;
	FText GetPossibleGrayscaleLevelsText() const;
	FText GetUniqueGrayscaleLevelsText() const;
	FText GetGrayscaleRangeText() const;
	FText GetHeightmapCompatibilityText() const;
	FText GetWorldText() const;
	FText GetLocalText() const;
	FText GetUvText() const;
	FText GetPixelText() const;
	FText GetStatusText() const;
	FText GetHeightZoneModeText() const;

	TWeakObjectPtr<ALandscapeProxy> AssignedLandscape;
	FLandscapeTrackerBounds LocalBounds;
	FLandscapeTrackerMappingResult LastMapping;
	FVector2D MarkerUV = FVector2D::ZeroVector;
	FVector2D HoverMarkerUV = FVector2D::ZeroVector;
	FVector2D LastLandscapeUV = FVector2D::ZeroVector;
	FIntPoint ImageSize = FIntPoint::ZeroValue;
	FString ImagePath;
	FString ImageFormat;
	int32 ImageBitDepth = 0;
	int32 PossibleGrayscaleLevelCount = 0;
	int32 UniqueGrayscaleLevelCount = 0;
	uint16 MinGrayscaleValue = 0;
	uint16 MaxGrayscaleValue = 0;
	TArray64<uint8> HeightmapGrayscaleData;
	TArray<float> HeightMetersCache;
	float MinHeightMeters = 0.0f;
	float MaxHeightMeters = 0.0f;
	FTransform CachedLandscapeTransform;
	FLandscapeTrackerBounds CachedHeightBounds;
	FIntPoint CachedHeightImageSize = FIntPoint::ZeroValue;
	int32 CachedHeightBitDepth = 0;
	bool bCachedFlipX = false;
	bool bCachedFlipY = true;
	FHeightZoneSettings HeightZoneSettings;
	FHeightZoneResult HeightZoneResult;
	TArray<TSharedPtr<EHeightZoneMode>> HeightZoneModeOptions;
	TSharedPtr<EHeightZoneMode> SelectedHeightZoneMode;
	FText StatusText;
	bool bHasHeightmapImageInfo = false;
	bool bSourceImageIsGrayscale = false;
	bool bHasMarker = false;
	bool bHasHoverMarker = false;
	bool bHoverTrackingHasViewportState = false;
	bool bHasLandscapeUV = false;
	bool bTrackClicks = false;
	bool bFlipX = false;
	bool bFlipY = true;
	bool bHeightMetersCacheValid = false;

	UTexture2D* HeightmapTexture = nullptr;
	UTexture2D* HeightZoneTexture = nullptr;
	FSlateBrush HeightmapBrush;
	FSlateBrush HeightZoneBrush;
	TSharedPtr<SWidget> HeightmapImageWidget;
	FDelegateHandle ClickDelegateHandle;
	FDelegateHandle HoverDelegateHandle;
};
