#if WITH_DEV_AUTOMATION_TESTS

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "LandscapeHeightmapTrackerModule.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"

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

#endif
