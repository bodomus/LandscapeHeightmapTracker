#include "LandscapeHeightmapTrackerStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FLandscapeHeightmapTrackerStyle::StyleInstance;

void FLandscapeHeightmapTrackerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLandscapeHeightmapTrackerStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

const ISlateStyle& FLandscapeHeightmapTrackerStyle::Get()
{
	return *StyleInstance;
}

FName FLandscapeHeightmapTrackerStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LandscapeHeightmapTrackerStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FLandscapeHeightmapTrackerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>(GetStyleSetName());
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LandscapeHeightmapTracker"));
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
		Style->Set(TEXT("LandscapeHeightmapTracker.OpenPluginWindow"), new FSlateImageBrush(Style->RootToContentDir(TEXT("Icon128.png")), FVector2D(40.0f, 40.0f)));
	}
	return Style;
}
