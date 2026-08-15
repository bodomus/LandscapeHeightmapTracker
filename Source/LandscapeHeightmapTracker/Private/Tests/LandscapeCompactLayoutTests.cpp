#if WITH_DEV_AUTOMATION_TESTS

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "SLandscapeHeightmapTrackerPanel.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLandscapePaintLayersTabSingletonTest,
	"LandscapeHeightmapTracker.UI.PaintLayersTabSingleton",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapePaintLayersTabSingletonTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddWarning(TEXT("Paint Layers tab singleton check requires a rendering-capable editor session."));
		return true;
	}

	const FString ScreenshotDirectory =
		FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("UE5-14"));
	IFileManager::Get().MakeDirectory(*ScreenshotDirectory, true);

	const auto CaptureWidget = [this, ScreenshotDirectory](
		const TSharedRef<SWidget>& Widget,
		const TCHAR* FileName,
		const TCHAR* Description)
	{
		Widget->SlatePrepass();
		TArray<FColor> ColorData;
		FIntVector ImageSize = FIntVector::ZeroValue;
		const bool bCaptured =
			FSlateApplication::Get().TakeScreenshot(Widget, ColorData, ImageSize);
		if (!TestTrue(Description, bCaptured))
		{
			return false;
		}

		const FString ScreenshotPath = FPaths::Combine(ScreenshotDirectory, FileName);
		return TestTrue(
			TEXT("Slate screenshot is saved"),
			FImageUtils::SaveImageByExtension(
				*ScreenshotPath,
				FImageView(ColorData.GetData(), ImageSize.X, ImageSize.Y)));
	};

	const TSharedPtr<SDockTab> MainTab =
		FGlobalTabmanager::Get()->TryInvokeTab(FLandscapeHeightmapTrackerModule::PluginTabName);
	if (!TestTrue(TEXT("Main Landscape Heightmap Tracker tab opens"), MainTab.IsValid()))
	{
		return false;
	}

	const TSharedPtr<SDockTab> FirstTab =
		FGlobalTabmanager::Get()->TryInvokeTab(FLandscapeHeightmapTrackerModule::PaintLayersTabName);
	if (!TestTrue(TEXT("Paint Layers tab opens through its registered spawner"), FirstTab.IsValid()))
	{
		return false;
	}

	const TSharedPtr<SDockTab> SecondTab =
		FGlobalTabmanager::Get()->TryInvokeTab(FLandscapeHeightmapTrackerModule::PaintLayersTabName);
	TestTrue(TEXT("Repeated invocation reuses the existing Paint Layers tab"), SecondTab.IsValid());
	TestTrue(TEXT("Repeated invocation does not create a duplicate tab"), FirstTab == SecondTab);

	AddCommand(new FWaitLatentCommand(1.0f));
	AddCommand(new FFunctionLatentCommand([CaptureWidget, MainTab, FirstTab]()
	{
		CaptureWidget(MainTab->GetContent(), TEXT("UE5-14-main-window.png"), TEXT("Main tab screenshot is captured"));
		CaptureWidget(FirstTab->GetContent(), TEXT("UE5-14-paint-layers-window.png"), TEXT("Paint Layers tab screenshot is captured"));

		if (FirstTab.IsValid())
		{
			FirstTab->RequestCloseTab();
		}
		if (MainTab.IsValid())
		{
			MainTab->RequestCloseTab();
		}
		return true;
	}));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLandscapeMultipleHeightRangesScreenshotTest,
	"LandscapeHeightmapTracker.UI.MultipleHeightRangesScreenshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLandscapeMultipleHeightRangesScreenshotTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddWarning(TEXT("Multiple height ranges screenshot requires a rendering-capable editor session."));
		return true;
	}

	constexpr int32 Width = 192;
	constexpr int32 Height = 192;
	TArray<float> HeightMeters;
	HeightMeters.SetNumUninitialized(Width * Height);
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const float NX = 2.0f * X / (Width - 1) - 1.0f;
			const float NY = 2.0f * Y / (Height - 1) - 1.0f;
			HeightMeters[Y * Width + X] =
				85.0f * NX + 35.0f * FMath::Sin(3.0f * NY) + 18.0f * FMath::Cos(5.0f * NX * NY);
		}
	}

	const auto MakeRange = [](double Min, double Max, const FLinearColor& Color)
	{
		FHeightRangeDefinition Range;
		Range.MinHeightMeters = Min;
		Range.MaxHeightMeters = Max;
		Range.Color = Color;
		return Range;
	};
	const TArray<FHeightRangeDefinition> Ranges =
	{
		MakeRange(-90.0, -35.0, FLinearColor(0.0f, 0.8f, 1.0f)),
		MakeRange(-35.0, 20.0, FLinearColor(0.1f, 0.9f, 0.2f)),
		MakeRange(20.0, 85.0, FLinearColor(1.0f, 0.1f, 0.75f))
	};

	const TSharedRef<SLandscapeHeightmapTrackerPanel> Panel = SNew(SLandscapeHeightmapTrackerPanel);
	FString Error;
	if (!TestTrue(
		TEXT("Synthetic multi-range preview is configured"),
		Panel->ConfigureMultiHeightRangePreviewForTesting(HeightMeters, Width, Height, Ranges, Error)))
	{
		AddError(Error);
		return false;
	}

	const TSharedRef<SWidget> FullUi =
		SNew(SBox)
		.WidthOverride(1200.0f)
		.HeightOverride(900.0f)
		[
			Panel
		];
	const TSharedRef<SWindow> PreviewWindow =
		SNew(SWindow)
		.Title(FText::FromString(TEXT("UE5-17 Multiple Height Ranges")))
		.ClientSize(FVector2D(1200.0f, 900.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		[
			FullUi
		];
	FSlateApplication::Get().AddWindow(PreviewWindow);
	const TSharedPtr<SWidget> OverlayWidget = Panel->GetHeightmapImageWidgetForTesting();
	if (!TestTrue(TEXT("Overlay widget is available"), OverlayWidget.IsValid()))
	{
		return false;
	}

	const FString ScreenshotDirectory =
		FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("UE5-17"));
	IFileManager::Get().MakeDirectory(*ScreenshotDirectory, true);
	AddCommand(new FWaitLatentCommand(1.0f));
	AddCommand(new FFunctionLatentCommand(
		[this, FullUi, PreviewWindow, ScreenshotDirectory]()
		{
			const auto Capture = [this, &ScreenshotDirectory](
				const TSharedRef<SWidget>& Widget,
				const TCHAR* FileName,
				const TCHAR* Description)
			{
				Widget->SlatePrepass();
				TArray<FColor> ColorData;
				FIntVector ImageSize = FIntVector::ZeroValue;
				const bool bCaptured = FSlateApplication::Get().TakeScreenshot(Widget, ColorData, ImageSize);
				if (!TestTrue(Description, bCaptured))
				{
					return false;
				}
				return TestTrue(
					TEXT("UE5-17 screenshot is saved"),
					FImageUtils::SaveImageByExtension(
						*FPaths::Combine(ScreenshotDirectory, FileName),
						FImageView(ColorData.GetData(), ImageSize.X, ImageSize.Y)));
			};

			Capture(FullUi, TEXT("UE5-17-height-range-ui.png"), TEXT("Height range UI screenshot is captured"));

			FullUi->SlatePrepass();
			TArray<FColor> FullColorData;
			FIntVector FullImageSize = FIntVector::ZeroValue;
			if (TestTrue(
				TEXT("Full UI is captured for the overlay crop"),
				FSlateApplication::Get().TakeScreenshot(FullUi, FullColorData, FullImageSize)))
			{
				const int32 CropX = FMath::Min(8, FullImageSize.X);
				const int32 CropHeight = FMath::Min(192, FullImageSize.Y);
				const int32 CropY = FullImageSize.Y - CropHeight;
				const int32 CropWidth = FMath::Min(514, FullImageSize.X - CropX);
				TArray<FColor> OverlayColorData;
				OverlayColorData.Reserve(CropWidth * CropHeight);
				for (int32 Y = 0; Y < CropHeight; ++Y)
				{
					const int32 SourceOffset = (CropY + Y) * FullImageSize.X + CropX;
					OverlayColorData.Append(FullColorData.GetData() + SourceOffset, CropWidth);
				}
				TestTrue(
					TEXT("Multi-color overlay screenshot is saved"),
					FImageUtils::SaveImageByExtension(
						*FPaths::Combine(ScreenshotDirectory, TEXT("UE5-17-multi-color-overlay.png")),
						FImageView(OverlayColorData.GetData(), CropWidth, CropHeight)));
			}
			PreviewWindow->RequestDestroyWindow();
			return true;
		}));

	return true;
}

#endif
