#include "SLandscapeHeightmapTrackerPanel.h"

#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HeightmapImageClickMapper.h"
#include "HeightmapImageInfoAnalyzer.h"
#include "IDesktopPlatform.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "InputCoreTypes.h"
#include "Landscape.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "LandscapeProxy.h"
#include "LandscapeSurfaceTraceHelper.h"
#include "LandscapeTrackerSettings.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "Rendering/DrawElements.h"
#include "Selection.h"
#include "Styling/AppStyle.h"
#include "TextureResource.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SLandscapeHeightmapTrackerPanel"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeHeightmapTrackerPanel, Log, All);

namespace
{
class SHeightmapTrackerImageView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHeightmapTrackerImageView) {}
		SLATE_ATTRIBUTE(const FSlateBrush*, ImageBrush)
		SLATE_ATTRIBUTE(FVector2D, MarkerUV)
		SLATE_ATTRIBUTE(bool, HasMarker)
		SLATE_ATTRIBUTE(FVector2D, HoverMarkerUV)
		SLATE_ATTRIBUTE(bool, HasHoverMarker)
		SLATE_EVENT(FSimpleDelegate, OnUnavailableClicked)
		SLATE_EVENT(FSimpleDelegate, OnOutsideImageClicked)
		SLATE_EVENT(TDelegate<void(FVector2D)>, OnHeightmapClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ImageBrush = InArgs._ImageBrush;
		MarkerUV = InArgs._MarkerUV;
		HasMarker = InArgs._HasMarker;
		HoverMarkerUV = InArgs._HoverMarkerUV;
		HasHoverMarker = InArgs._HasHoverMarker;
		OnUnavailableClicked = InArgs._OnUnavailableClicked;
		OnOutsideImageClicked = InArgs._OnOutsideImageClicked;
		OnHeightmapClicked = InArgs._OnHeightmapClicked;
	}

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		return FVector2D(512.0f, 512.0f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FSlateBrush* Brush = ImageBrush.Get();
		if (!Brush || Brush->GetResourceObject() == nullptr || Brush->ImageSize.X <= 0.0f || Brush->ImageSize.Y <= 0.0f)
		{
			return LayerId;
		}

		const FHeightmapFittedImageRect ImageRect = FHeightmapImageClickMapper::CalculateFittedImageRect(
			AllottedGeometry.GetLocalSize(),
			Brush->ImageSize);
		if (!ImageRect.bIsValid)
		{
			return LayerId;
		}

		const FPaintGeometry ImageGeometry = AllottedGeometry.ToPaintGeometry(ImageRect.DrawSize, FSlateLayoutTransform(ImageRect.DrawOffset));

		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, ImageGeometry, Brush, ESlateDrawEffect::None, InWidgetStyle.GetColorAndOpacityTint());

		const bool bUseHoverMarker = HasHoverMarker.Get();
		const bool bUseClickMarker = !bUseHoverMarker && HasMarker.Get();
		if (bUseHoverMarker || bUseClickMarker)
		{
			const FVector2D UV = bUseHoverMarker ? HoverMarkerUV.Get() : MarkerUV.Get();
			const FVector2D MarkerCenter = ImageRect.DrawOffset + FVector2D(UV.X * ImageRect.DrawSize.X, UV.Y * ImageRect.DrawSize.Y);
			const float Radius = 8.0f;
			const FLinearColor Outer = FLinearColor::Black;
			const FLinearColor Inner = FLinearColor::Yellow;

			TArray<FVector2D> Horizontal;
			Horizontal.Add(MarkerCenter + FVector2D(-Radius * 1.5f, 0.0f));
			Horizontal.Add(MarkerCenter + FVector2D(Radius * 1.5f, 0.0f));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Horizontal, ESlateDrawEffect::None, Outer, true, 3.0f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Horizontal, ESlateDrawEffect::None, Inner, true, 1.0f);

			TArray<FVector2D> Vertical;
			Vertical.Add(MarkerCenter + FVector2D(0.0f, -Radius * 1.5f));
			Vertical.Add(MarkerCenter + FVector2D(0.0f, Radius * 1.5f));
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Vertical, ESlateDrawEffect::None, Outer, true, 3.0f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Vertical, ESlateDrawEffect::None, Inner, true, 1.0f);
		}

		return LayerId + 2;
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		{
			return FReply::Unhandled();
		}

		const FSlateBrush* Brush = ImageBrush.Get();
		if (!Brush || Brush->GetResourceObject() == nullptr || Brush->ImageSize.X <= 0.0f || Brush->ImageSize.Y <= 0.0f)
		{
			OnUnavailableClicked.ExecuteIfBound();
			return FReply::Handled();
		}

		const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const FHeightmapFittedImageRect ImageRect = FHeightmapImageClickMapper::CalculateFittedImageRect(MyGeometry.GetLocalSize(), Brush->ImageSize);
		const FHeightmapClickMappingResult ClickMapping = FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(LocalPosition, ImageRect);

		if (ClickMapping.bIsValid)
		{
			OnHeightmapClicked.ExecuteIfBound(ClickMapping.DisplayUV);
		}
		else
		{
			OnOutsideImageClicked.ExecuteIfBound();
		}

		return FReply::Handled();
	}

private:
	TAttribute<const FSlateBrush*> ImageBrush;
	TAttribute<FVector2D> MarkerUV;
	TAttribute<bool> HasMarker;
	TAttribute<FVector2D> HoverMarkerUV;
	TAttribute<bool> HasHoverMarker;
	FSimpleDelegate OnUnavailableClicked;
	FSimpleDelegate OnOutsideImageClicked;
	TDelegate<void(FVector2D)> OnHeightmapClicked;
};

TSharedRef<SWidget> MakeLabelValue(const FText& Label, TAttribute<FText> Value)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock).Text(Label).Font(FAppStyle::GetFontStyle("SmallFontBold"))
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 2.0f)
		[
			SNew(STextBlock).Text(Value)
		];
}
}

void SLandscapeHeightmapTrackerPanel::Construct(const FArguments& InArgs)
{
	ULandscapeTrackerSettings* Settings = GetMutableDefault<ULandscapeTrackerSettings>();
	bFlipX = Settings->bFlipX;
	bFlipY = Settings->bFlipY;
	bTrackClicks = Settings->bTrackClicks;
	ImagePath = Settings->LastHeightmapPath;
	UpdateStatus(LOCTEXT("InitialStatus", "No Landscape assigned. Select a Landscape and click \"Use Selected Landscape\"."));

	HeightmapBrush.DrawAs = ESlateBrushDrawType::Image;
	HeightmapBrush.Tiling = ESlateBrushTileType::NoTile;

	ClickDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
	HoverDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportHoverResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportHover);
	FLandscapeHeightmapTrackerModule::SetTrackingModeEnabled(bTrackClicks);

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[
				SNew(STextBlock).Text(FText::Format(LOCTEXT("LandscapeHeader", "Landscape {0}"), FText::FromString(FLandscapeHeightmapTrackerModule::GetPluginVersion()))).Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(ALandscapeProxy::StaticClass())
				.ObjectPath_Lambda([this]() { return AssignedLandscape.IsValid() ? AssignedLandscape->GetPathName() : FString(); })
				.OnObjectChanged(this, &SLandscapeHeightmapTrackerPanel::OnObjectSelected)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("UseSelectedLandscape", "Use Selected Landscape"))
				.OnClicked(this, &SLandscapeHeightmapTrackerPanel::UseSelectedLandscape)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[
				MakeLabelValue(LOCTEXT("LandscapeName", "Selected:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetLandscapeNameText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Location", "Location:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetActorLocationText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Rotation", "Rotation:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetActorRotationText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Scale", "Scale:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetActorScaleText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Bounds", "Local XY bounds:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetLocalBoundsText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f, 8.0f, 8.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("HeightmapHeader", "Heightmap")).Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("LoadHeightmap", "Load Heightmap..."))
				.OnClicked(this, &SLandscapeHeightmapTrackerPanel::LoadHeightmap)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[
				MakeLabelValue(LOCTEXT("ImagePath", "Source:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetImagePathText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("ImageInfo", "Image:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetImageInfoText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 8.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
				.Padding(8.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock).Text(LOCTEXT("HeightmapInformationHeader", "Heightmap Information")).Font(FAppStyle::GetFontStyle("SmallFontBold"))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapColorModel", "Color model:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetImageColorModelText))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapBitDepth", "Bit depth:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetImageBitDepthText))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapPossibleLevels", "Possible grayscale levels:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetPossibleGrayscaleLevelsText))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapUniqueLevels", "Unique grayscale levels:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetUniqueGrayscaleLevelsText))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapValueRange", "Value range:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetGrayscaleRangeText))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeLabelValue(LOCTEXT("HeightmapCompatibility", "UE Landscape:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetHeightmapCompatibilityText))
					]
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.0f).MinHeight(320.0f).Padding(8.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
				[
					SAssignNew(HeightmapImageWidget, SHeightmapTrackerImageView)
					.ImageBrush_Lambda([this]() { return HeightmapTexture ? &HeightmapBrush : nullptr; })
					.MarkerUV_Lambda([this]() { return MarkerUV; })
					.HasMarker_Lambda([this]() { return bHasMarker && !bHoverTrackingHasViewportState; })
					.HoverMarkerUV_Lambda([this]() { return HoverMarkerUV; })
					.HasHoverMarker_Lambda([this]() { return bHasHoverMarker; })
					.OnUnavailableClicked(FSimpleDelegate::CreateLambda([this]() { UpdateStatus(LOCTEXT("NoHeightmapLoadedClick", "No heightmap loaded.")); }))
					.OnOutsideImageClicked(FSimpleDelegate::CreateLambda([this]() { UpdateStatus(LOCTEXT("OutsideImageClick", "Click is outside the heightmap image area.")); }))
					.OnHeightmapClicked(TDelegate<void(FVector2D)>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::OnHeightmapClicked))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f, 8.0f, 8.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("TrackingHeader", "Tracking")).Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SLandscapeHeightmapTrackerPanel::IsTrackingChecked)
				.OnCheckStateChanged(this, &SLandscapeHeightmapTrackerPanel::SetTrackingEnabled)
				[
					SNew(STextBlock).Text(LOCTEXT("TrackLandscapeClicks", "Track Landscape Clicks"))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 16.0f, 0.0f)
				[
					SNew(SCheckBox)
					.IsChecked(this, &SLandscapeHeightmapTrackerPanel::IsFlipXChecked)
					.OnCheckStateChanged(this, &SLandscapeHeightmapTrackerPanel::SetFlipX)
					[
						SNew(STextBlock).Text(LOCTEXT("FlipX", "Flip X"))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(this, &SLandscapeHeightmapTrackerPanel::IsFlipYChecked)
					.OnCheckStateChanged(this, &SLandscapeHeightmapTrackerPanel::SetFlipY)
					[
						SNew(STextBlock).Text(LOCTEXT("FlipY", "Flip Y"))
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearMarker", "Clear All Markers"))
				.OnClicked(this, &SLandscapeHeightmapTrackerPanel::ClearMarker)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f, 8.0f, 8.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("DiagnosticsHeader", "Coordinate Diagnostics")).Font(FAppStyle::GetFontStyle("HeadingMedium"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("World", "World:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetWorldText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Local", "Landscape:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetLocalText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("UV", "UV:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetUvText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
			[
				MakeLabelValue(LOCTEXT("Pixel", "Pixel:"), TAttribute<FText>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::GetPixelText))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(this, &SLandscapeHeightmapTrackerPanel::GetStatusText)
				.AutoWrapText(true)
			]
		]
	];

	if (!ImagePath.IsEmpty() && FPaths::FileExists(ImagePath))
	{
		FString Error;
		LoadPngTexture(ImagePath, Error);
	}
}

SLandscapeHeightmapTrackerPanel::~SLandscapeHeightmapTrackerPanel()
{
	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Remove(ClickDelegateHandle);
	FLandscapeHeightmapTrackerModule::OnViewportHoverResult().Remove(HoverDelegateHandle);
	ClearHoverMarker();
	FLandscapeHeightmapTrackerModule::SetTrackingModeEnabled(false);
	FLandscapeHeightmapTrackerModule::ClearReverseMarker();
	ReleaseTexture();
}

FReply SLandscapeHeightmapTrackerPanel::UseSelectedLandscape()
{
	if (!GEditor)
	{
		UpdateStatus(LOCTEXT("NoEditor", "Editor selection is unavailable."));
		return FReply::Handled();
	}

	TArray<ALandscapeProxy*> SelectedLandscapes;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(*It))
		{
			SelectedLandscapes.Add(Landscape);
		}
	}

	if (SelectedLandscapes.Num() == 1)
	{
		AssignLandscape(SelectedLandscapes[0]);
	}
	else if (SelectedLandscapes.Num() == 0)
	{
		AssignLandscape(nullptr);
		UpdateStatus(LOCTEXT("NoLandscapeSelected", "No Landscape selected. Select exactly one Landscape and click \"Use Selected Landscape\"."));
	}
	else
	{
		AssignLandscape(nullptr);
		UpdateStatus(LOCTEXT("MultipleLandscapesSelected", "Multiple Landscapes selected. Select exactly one Landscape."));
	}

	return FReply::Handled();
}

FReply SLandscapeHeightmapTrackerPanel::LoadHeightmap()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UpdateStatus(LOCTEXT("NoDesktopPlatform", "Heightmap could not be loaded: desktop file picker is unavailable."));
		return FReply::Handled();
	}

	const ULandscapeTrackerSettings* Settings = GetDefault<ULandscapeTrackerSettings>();
	FString DefaultPath = Settings->LastHeightmapDirectory;
	TArray<FString> OpenedFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Load Heightmap"),
		DefaultPath,
		TEXT(""),
		TEXT("PNG files (*.png)|*.png"),
		EFileDialogFlags::None,
		OpenedFiles);

	if (bOpened && OpenedFiles.Num() > 0)
	{
		FString Error;
		if (!LoadPngTexture(OpenedFiles[0], Error))
		{
			UpdateStatus(FText::Format(LOCTEXT("LoadFailed", "Heightmap could not be loaded: {0}"), FText::FromString(Error)));
		}
	}

	return FReply::Handled();
}

FReply SLandscapeHeightmapTrackerPanel::ClearMarker()
{
	bHasMarker = false;
	bHoverTrackingHasViewportState = false;
	bHasLandscapeUV = false;
	ClearHoverMarker();
	LastMapping = FLandscapeTrackerMappingResult();
	LastLandscapeUV = FVector2D::ZeroVector;
	FLandscapeHeightmapTrackerModule::ClearReverseMarker();
	InvalidateHeightmapMarkerPaint();
	UpdateStatus(LOCTEXT("MarkerCleared", "All markers cleared."));
	return FReply::Handled();
}

void SLandscapeHeightmapTrackerPanel::OnObjectSelected(const FAssetData& AssetData)
{
	AssignLandscape(Cast<ALandscapeProxy>(AssetData.GetAsset()));
}

void SLandscapeHeightmapTrackerPanel::OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click)
{
	if (!bTrackClicks)
	{
		return;
	}

	if (!AssignedLandscape.IsValid())
	{
		bHasMarker = false;
		if (!bHasHoverMarker)
		{
			InvalidateHeightmapMarkerPaint();
		}
		UpdateStatus(LOCTEXT("NoLandscapeAssignedClick", "No Landscape assigned."));
		return;
	}

	if (!IsAssignedLandscapeHit(Click.HitActor.Get(), Click.HitComponent.Get()))
	{
		UpdateStatus(LOCTEXT("WrongActorHit", "Click ignored: hit actor is not the assigned Landscape."));
		return;
	}

	RefreshLandscapeBounds();
	FLandscapeTrackerMappingOptions Options;
	Options.bFlipX = bFlipX;
	Options.bFlipY = bFlipY;

	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
	if (LastMapping.bIsValid)
	{
		bHasMarker = true;
		bHoverTrackingHasViewportState = false;
		bHasLandscapeUV = false;
		MarkerUV = LastMapping.NormalizedUV;
		if (!bHasHoverMarker)
		{
			InvalidateHeightmapMarkerPaint();
		}
		UpdateStatus(LOCTEXT("MappedClick", "Landscape click mapped to heightmap."));
		UE_LOG(LogLandscapeHeightmapTrackerPanel, Log, TEXT("Valid click mapped to U=%f V=%f Pixel=(%d,%d)."), LastMapping.NormalizedUV.X, LastMapping.NormalizedUV.Y, LastMapping.Pixel.X, LastMapping.Pixel.Y);
	}
	else
	{
		bHasMarker = false;
		if (!bHasHoverMarker)
		{
			InvalidateHeightmapMarkerPaint();
		}
		UpdateStatus(FText::FromString(LastMapping.FailureReason));
	}
}

void SLandscapeHeightmapTrackerPanel::OnViewportHover(const FLandscapeHeightmapTrackerModule::FViewportHoverResult& Hover)
{
	if (!bTrackClicks)
	{
		bHoverTrackingHasViewportState = false;
		ClearHoverMarker();
		return;
	}

	const bool bDisplayStateChanged = !bHoverTrackingHasViewportState;
	bHoverTrackingHasViewportState = true;
	if (!AssignedLandscape.IsValid() || !Hover.bHasHit)
	{
		ClearHoverMarker();
		if (bDisplayStateChanged)
		{
			InvalidateHeightmapMarkerPaint();
		}
		return;
	}

	if (!IsAssignedLandscapeHit(Hover.HitActor.Get(), Hover.HitComponent.Get()))
	{
		ClearHoverMarker();
		if (bDisplayStateChanged)
		{
			InvalidateHeightmapMarkerPaint();
		}
		return;
	}

	RefreshLandscapeBounds();

	FLandscapeTrackerMappingOptions Options;
	Options.bFlipX = bFlipX;
	Options.bFlipY = bFlipY;

	const FLandscapeTrackerMappingResult HoverMapping = FLandscapeCoordinateMapper::MapWorldPosition(
		AssignedLandscape->GetActorTransform(),
		LocalBounds,
		Hover.WorldPosition,
		ImageSize,
		Options);
	if (!HoverMapping.bIsValid)
	{
		ClearHoverMarker();
		if (bDisplayStateChanged)
		{
			InvalidateHeightmapMarkerPaint();
		}
		return;
	}

	SetHoverMarkerUV(HoverMapping.NormalizedUV);
}

void SLandscapeHeightmapTrackerPanel::OnHeightmapClicked(FVector2D DisplayUV)
{
	if (!AssignedLandscape.IsValid())
	{
		UpdateStatus(LOCTEXT("NoLandscapeAssignedHeightmapClick", "No Landscape assigned."));
		return;
	}

	if (!HeightmapTexture || ImageSize.X <= 0 || ImageSize.Y <= 0)
	{
		UpdateStatus(LOCTEXT("NoHeightmapLoadedClick", "No heightmap loaded."));
		return;
	}

	RefreshLandscapeBounds();

	FLandscapeTrackerMappingOptions Options;
	Options.bFlipX = bFlipX;
	Options.bFlipY = bFlipY;

	const FLandscapeTrackerReverseMappingResult ReverseMapping = FLandscapeCoordinateMapper::MapUVToLocalPosition(LocalBounds, DisplayUV, Options);
	if (!ReverseMapping.bIsValid)
	{
		UpdateStatus(FText::FromString(ReverseMapping.FailureReason));
		return;
	}

	const FTransform& LandscapeTransform = AssignedLandscape->GetActorTransform();
	const FVector WorldXY = LandscapeTransform.TransformPosition(ReverseMapping.LocalPosition);
	UWorld* World = AssignedLandscape->GetWorld();
	const FLandscapeSurfaceTraceResult SurfaceTrace = FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface(World, AssignedLandscape.Get(), WorldXY);
	if (!SurfaceTrace.bIsValid)
	{
		UpdateStatus(FText::FromString(SurfaceTrace.FailureReason));
		return;
	}

	LastMapping = FLandscapeTrackerMappingResult();
	LastMapping.bIsValid = true;
	LastMapping.bIsInsideBounds = true;
	LastMapping.WorldPosition = SurfaceTrace.WorldPosition;
	LastMapping.LocalPosition = ReverseMapping.LocalPosition;
	LastMapping.NormalizedUV = ReverseMapping.DisplayUV;
	LastMapping.Pixel = FLandscapeCoordinateMapper::UVToPixel(ReverseMapping.DisplayUV, ImageSize);
	LastLandscapeUV = ReverseMapping.LandscapeUV;
	bHasLandscapeUV = true;

	FLandscapeHeightmapTrackerModule::SetReverseMarker(SurfaceTrace.WorldPosition, AssignedLandscape.Get());
	UpdateStatus(LOCTEXT("MappedHeightmapClick", "Heightmap click mapped to Landscape surface."));

	UE_LOG(
		LogLandscapeHeightmapTrackerPanel,
		Verbose,
		TEXT("Heightmap click mapped. DisplayUV=(%f,%f) LandscapeUV=(%f,%f) Local=%s WorldXY=%s Surface=%s HitCount=%d"),
		ReverseMapping.DisplayUV.X,
		ReverseMapping.DisplayUV.Y,
		ReverseMapping.LandscapeUV.X,
		ReverseMapping.LandscapeUV.Y,
		*ReverseMapping.LocalPosition.ToCompactString(),
		*WorldXY.ToCompactString(),
		*SurfaceTrace.WorldPosition.ToCompactString(),
		SurfaceTrace.HitCount);
}

void SLandscapeHeightmapTrackerPanel::SetTrackingEnabled(ECheckBoxState NewState)
{
	bTrackClicks = NewState == ECheckBoxState::Checked;
	if (!bTrackClicks)
	{
		bHoverTrackingHasViewportState = false;
		ClearHoverMarker();
		InvalidateHeightmapMarkerPaint();
	}
	FLandscapeHeightmapTrackerModule::SetTrackingModeEnabled(bTrackClicks);

	ULandscapeTrackerSettings* Settings = GetMutableDefault<ULandscapeTrackerSettings>();
	Settings->bTrackClicks = bTrackClicks;
	Settings->SaveConfig();

	UpdateStatus(bTrackClicks ? LOCTEXT("TrackingEnabled", "Tracking enabled.") : LOCTEXT("TrackingDisabled", "Tracking disabled."));
}

void SLandscapeHeightmapTrackerPanel::SetFlipX(ECheckBoxState NewState)
{
	bFlipX = NewState == ECheckBoxState::Checked;
	ULandscapeTrackerSettings* Settings = GetMutableDefault<ULandscapeTrackerSettings>();
	Settings->bFlipX = bFlipX;
	Settings->SaveConfig();
}

void SLandscapeHeightmapTrackerPanel::SetFlipY(ECheckBoxState NewState)
{
	bFlipY = NewState == ECheckBoxState::Checked;
	ULandscapeTrackerSettings* Settings = GetMutableDefault<ULandscapeTrackerSettings>();
	Settings->bFlipY = bFlipY;
	Settings->SaveConfig();
}

void SLandscapeHeightmapTrackerPanel::AssignLandscape(ALandscapeProxy* InLandscape)
{
	AssignedLandscape = InLandscape;
	bHasMarker = false;
	bHoverTrackingHasViewportState = false;
	bHasLandscapeUV = false;
	ClearHoverMarker();
	FLandscapeHeightmapTrackerModule::ClearReverseMarker();
	RefreshLandscapeBounds();
	InvalidateHeightmapMarkerPaint();
	if (AssignedLandscape.IsValid())
	{
		UpdateStatus(FText::Format(LOCTEXT("AssignedLandscape", "Assigned Landscape: {0}"), FText::FromString(AssignedLandscape->GetName())));
		UE_LOG(LogLandscapeHeightmapTrackerPanel, Log, TEXT("Assigned Landscape %s."), *AssignedLandscape->GetName());
	}
}

void SLandscapeHeightmapTrackerPanel::RefreshLandscapeBounds()
{
	LocalBounds = FLandscapeTrackerBounds();
	if (!AssignedLandscape.IsValid())
	{
		return;
	}

	const FIntRect Rect = AssignedLandscape->GetBoundingRect();
	LocalBounds.Min = FVector2D(Rect.Min.X, Rect.Min.Y);
	LocalBounds.Max = FVector2D(Rect.Max.X, Rect.Max.Y);
}

void SLandscapeHeightmapTrackerPanel::ReleaseTexture()
{
	HeightmapBrush.SetResourceObject(nullptr);
	bHasHeightmapImageInfo = false;
	bSourceImageIsGrayscale = false;
	ImageBitDepth = 0;
	PossibleGrayscaleLevelCount = 0;
	UniqueGrayscaleLevelCount = 0;
	MinGrayscaleValue = 0;
	MaxGrayscaleValue = 0;
	bHasMarker = false;
	bHoverTrackingHasViewportState = false;
	ClearHoverMarker();
	InvalidateHeightmapMarkerPaint();
	if (HeightmapTexture)
	{
		HeightmapTexture->RemoveFromRoot();
		HeightmapTexture = nullptr;
	}
}

bool SLandscapeHeightmapTrackerPanel::LoadPngTexture(const FString& FilePath, FString& OutError)
{
	TArray64<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *FilePath))
	{
		OutError = TEXT("file could not be read.");
		return false;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	const EImageFormat ImageFormatValue = ImageWrapperModule.DetectImageFormat(CompressedData.GetData(), CompressedData.Num());
	if (ImageFormatValue != EImageFormat::PNG)
	{
		OutError = TEXT("only PNG files are supported.");
		return false;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormatValue);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
	{
		OutError = TEXT("PNG decoder failed.");
		return false;
	}

	const int32 SourceBitDepth = ImageWrapper->GetBitDepth();
	const bool bSourceIsGrayscale = ImageWrapper->GetFormat() == ERGBFormat::Gray;
	TArray64<uint8> GrayscaleData;
	if (!ImageWrapper->GetRaw(ERGBFormat::Gray, SourceBitDepth, GrayscaleData))
	{
		OutError = TEXT("PNG grayscale samples could not be decoded for analysis.");
		return false;
	}

	const FHeightmapGrayscaleInfo GrayscaleInfo = FHeightmapImageInfoAnalyzer::Analyze(GrayscaleData, SourceBitDepth);
	if (!GrayscaleInfo.bIsValid)
	{
		OutError = TEXT("PNG grayscale information could not be analyzed.");
		return false;
	}

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		OutError = TEXT("PNG could not be converted to BGRA8 for display.");
		return false;
	}

	ReleaseTexture();

	ImageSize = FIntPoint(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
	HeightmapTexture = UTexture2D::CreateTransient(ImageSize.X, ImageSize.Y, PF_B8G8R8A8);
	if (!HeightmapTexture)
	{
		OutError = TEXT("transient texture allocation failed.");
		return false;
	}

	HeightmapTexture->AddToRoot();
	HeightmapTexture->SRGB = false;
	void* TextureData = HeightmapTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	HeightmapTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	HeightmapTexture->UpdateResource();

	HeightmapBrush.SetResourceObject(HeightmapTexture);
	HeightmapBrush.SetImageSize(FVector2D(ImageSize.X, ImageSize.Y));

	ImagePath = FilePath;
	ImageFormat = TEXT("PNG");
	bHasHeightmapImageInfo = true;
	bSourceImageIsGrayscale = bSourceIsGrayscale;
	ImageBitDepth = GrayscaleInfo.BitDepth;
	PossibleGrayscaleLevelCount = GrayscaleInfo.PossibleLevelCount;
	UniqueGrayscaleLevelCount = GrayscaleInfo.UniqueLevelCount;
	MinGrayscaleValue = GrayscaleInfo.MinValue;
	MaxGrayscaleValue = GrayscaleInfo.MaxValue;
	bHasMarker = false;
	bHoverTrackingHasViewportState = false;
	ClearHoverMarker();
	InvalidateHeightmapMarkerPaint();

	ULandscapeTrackerSettings* Settings = GetMutableDefault<ULandscapeTrackerSettings>();
	Settings->LastHeightmapPath = FilePath;
	Settings->LastHeightmapDirectory = FPaths::GetPath(FilePath);
	Settings->SaveConfig();

	UpdateStatus(FText::Format(LOCTEXT("HeightmapLoaded", "Heightmap loaded: {0} x {1} PNG."), FText::AsNumber(ImageSize.X), FText::AsNumber(ImageSize.Y)));
	UE_LOG(LogLandscapeHeightmapTrackerPanel, Log, TEXT("Loaded heightmap %s (%d x %d)."), *FilePath, ImageSize.X, ImageSize.Y);
	return true;
}

bool SLandscapeHeightmapTrackerPanel::IsAssignedLandscapeHit(AActor* HitActor, UPrimitiveComponent* HitComponent) const
{
	if (!AssignedLandscape.IsValid())
	{
		return false;
	}

	return FLandscapeSurfaceTraceHelper::HitBelongsToAssignedLandscape(AssignedLandscape.Get(), HitActor, HitComponent);
}

void SLandscapeHeightmapTrackerPanel::ClearHoverMarker()
{
	if (!bHasHoverMarker)
	{
		return;
	}

	bHasHoverMarker = false;
	HoverMarkerUV = FVector2D::ZeroVector;
	InvalidateHeightmapMarkerPaint();
}

bool SLandscapeHeightmapTrackerPanel::SetHoverMarkerUV(const FVector2D& NewUV)
{
	if (bHasHoverMarker && HoverMarkerUV.Equals(NewUV, KINDA_SMALL_NUMBER))
	{
		return false;
	}

	bHasHoverMarker = true;
	HoverMarkerUV = NewUV;
	InvalidateHeightmapMarkerPaint();
	return true;
}

void SLandscapeHeightmapTrackerPanel::InvalidateHeightmapMarkerPaint()
{
	if (HeightmapImageWidget.IsValid())
	{
		HeightmapImageWidget->Invalidate(EInvalidateWidgetReason::Paint);
		return;
	}

	Invalidate(EInvalidateWidgetReason::Paint);
}

void SLandscapeHeightmapTrackerPanel::UpdateStatus(const FText& InStatus)
{
	StatusText = InStatus;
}

ECheckBoxState SLandscapeHeightmapTrackerPanel::IsTrackingChecked() const { return bTrackClicks ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }
ECheckBoxState SLandscapeHeightmapTrackerPanel::IsFlipXChecked() const { return bFlipX ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }
ECheckBoxState SLandscapeHeightmapTrackerPanel::IsFlipYChecked() const { return bFlipY ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; }

FText SLandscapeHeightmapTrackerPanel::GetLandscapeNameText() const { return AssignedLandscape.IsValid() ? FText::FromString(AssignedLandscape->GetName()) : LOCTEXT("None", "None"); }
FText SLandscapeHeightmapTrackerPanel::GetActorLocationText() const { return AssignedLandscape.IsValid() ? FText::FromString(AssignedLandscape->GetActorLocation().ToCompactString()) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetActorRotationText() const { return AssignedLandscape.IsValid() ? FText::FromString(AssignedLandscape->GetActorRotation().ToCompactString()) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetActorScaleText() const { return AssignedLandscape.IsValid() ? FText::FromString(AssignedLandscape->GetActorScale3D().ToCompactString()) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetLocalBoundsText() const { return LocalBounds.IsValid() ? FText::Format(LOCTEXT("BoundsFormat", "Min=({0}, {1}) Max=({2}, {3})"), FText::AsNumber(LocalBounds.Min.X), FText::AsNumber(LocalBounds.Min.Y), FText::AsNumber(LocalBounds.Max.X), FText::AsNumber(LocalBounds.Max.Y)) : LOCTEXT("InvalidBounds", "Unavailable"); }
FText SLandscapeHeightmapTrackerPanel::GetImagePathText() const { return ImagePath.IsEmpty() ? LOCTEXT("NoImage", "No heightmap loaded.") : FText::FromString(ImagePath); }
FText SLandscapeHeightmapTrackerPanel::GetImageInfoText() const { return ImageSize.X > 0 ? FText::Format(LOCTEXT("ImageInfoFormat", "{0} x {1} {2}"), FText::AsNumber(ImageSize.X), FText::AsNumber(ImageSize.Y), FText::FromString(ImageFormat)) : LOCTEXT("NoImageInfo", "No image."); }
FText SLandscapeHeightmapTrackerPanel::GetImageColorModelText() const
{
	if (!bHasHeightmapImageInfo)
	{
		return LOCTEXT("NoHeightmapColorModel", "Unavailable");
	}

	return bSourceImageIsGrayscale
		? LOCTEXT("GrayscaleColorModel", "Grayscale (single channel)")
		: LOCTEXT("ConvertedGrayscaleColorModel", "Color source (values converted to grayscale)");
}
FText SLandscapeHeightmapTrackerPanel::GetImageBitDepthText() const
{
	return bHasHeightmapImageInfo
		? FText::Format(LOCTEXT("HeightmapBitDepthFormat", "{0}-bit per channel"), FText::AsNumber(ImageBitDepth))
		: LOCTEXT("NoHeightmapBitDepth", "Unavailable");
}
FText SLandscapeHeightmapTrackerPanel::GetPossibleGrayscaleLevelsText() const
{
	return bHasHeightmapImageInfo ? FText::AsNumber(PossibleGrayscaleLevelCount) : LOCTEXT("NoPossibleGrayscaleLevels", "Unavailable");
}
FText SLandscapeHeightmapTrackerPanel::GetUniqueGrayscaleLevelsText() const
{
	return bHasHeightmapImageInfo ? FText::AsNumber(UniqueGrayscaleLevelCount) : LOCTEXT("NoUniqueGrayscaleLevels", "Unavailable");
}
FText SLandscapeHeightmapTrackerPanel::GetGrayscaleRangeText() const
{
	return bHasHeightmapImageInfo
		? FText::Format(LOCTEXT("GrayscaleRangeFormat", "{0} - {1}"), FText::AsNumber(MinGrayscaleValue), FText::AsNumber(MaxGrayscaleValue))
		: LOCTEXT("NoGrayscaleRange", "Unavailable");
}
FText SLandscapeHeightmapTrackerPanel::GetHeightmapCompatibilityText() const
{
	if (!bHasHeightmapImageInfo)
	{
		return LOCTEXT("NoHeightmapCompatibility", "Unavailable");
	}

	return bSourceImageIsGrayscale && ImageBitDepth == 16
		? LOCTEXT("RecommendedHeightmapCompatibility", "Recommended: 16-bit grayscale PNG")
		: LOCTEXT("NonRecommendedHeightmapCompatibility", "Not recommended: UE Landscape expects 16-bit grayscale PNG");
}
FText SLandscapeHeightmapTrackerPanel::GetWorldText() const { return LastMapping.bIsValid ? FText::FromString(LastMapping.WorldPosition.ToCompactString()) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetLocalText() const { return LastMapping.bIsValid ? FText::FromString(LastMapping.LocalPosition.ToCompactString()) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetUvText() const
{
	if (!LastMapping.bIsValid)
	{
		return FText::GetEmpty();
	}

	if (bHasLandscapeUV)
	{
		return FText::Format(
			LOCTEXT("ReverseUvFormat", "Display U={0} V={1}; Landscape U={2} V={3}"),
			FText::AsNumber(LastMapping.NormalizedUV.X),
			FText::AsNumber(LastMapping.NormalizedUV.Y),
			FText::AsNumber(LastLandscapeUV.X),
			FText::AsNumber(LastLandscapeUV.Y));
	}

	return FText::Format(LOCTEXT("UvFormat", "U={0} V={1}"), FText::AsNumber(LastMapping.NormalizedUV.X), FText::AsNumber(LastMapping.NormalizedUV.Y));
}
FText SLandscapeHeightmapTrackerPanel::GetPixelText() const { return LastMapping.bIsValid ? FText::Format(LOCTEXT("PixelFormat", "X={0} Y={1}"), FText::AsNumber(LastMapping.Pixel.X), FText::AsNumber(LastMapping.Pixel.Y)) : FText::GetEmpty(); }
FText SLandscapeHeightmapTrackerPanel::GetStatusText() const { return StatusText; }

#undef LOCTEXT_NAMESPACE
