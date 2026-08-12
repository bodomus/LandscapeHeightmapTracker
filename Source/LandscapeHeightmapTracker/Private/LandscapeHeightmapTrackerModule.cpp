#include "LandscapeHeightmapTrackerModule.h"

#include "EditorModeManager.h"
#include "EditorModeRegistry.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IPluginManager.h"
#include "LandscapeHeightmapTrackerCommands.h"
#include "LandscapeHeightmapTrackerEdMode.h"
#include "LandscapeHeightmapTrackerStyle.h"
#include "LevelEditor.h"
#include "SLandscapePaintLayerBulkRemoveWidget.h"
#include "SLandscapeHeightmapTrackerPanel.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FLandscapeHeightmapTrackerModule"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeHeightmapTracker, Log, All);

const FName FLandscapeHeightmapTrackerModule::PluginTabName(TEXT("LandscapeHeightmapTracker"));
const FName FLandscapeHeightmapTrackerModule::PaintLayersTabName(TEXT("LandscapeHeightmapTracker.PaintLayers"));
const FName FLandscapeHeightmapTrackerModule::EditorModeId(TEXT("EM_LandscapeHeightmapTracker"));

static FLandscapeHeightmapTrackerModule::FOnViewportClickResult GOnViewportClickResult;
static FLandscapeHeightmapTrackerModule::FOnViewportHoverResult GOnViewportHoverResult;
static bool GIsTrackingModeEnabled = false;
static bool GHasReverseMarker = false;
static bool GReverseMarkerNeedsCleanup = false;
static FVector GReverseMarkerWorldPosition = FVector::ZeroVector;
static TWeakObjectPtr<AActor> GReverseMarkerOwner;

namespace
{
	FString ReadPluginVersion()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LandscapeHeightmapTracker"));
		if (!Plugin.IsValid())
		{
			return TEXT("Unknown");
		}

		FString VersionJsonContents;
		const FString VersionJsonPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("version.json"));
		if (!FFileHelper::LoadFileToString(VersionJsonContents, *VersionJsonPath))
		{
			return TEXT("Unknown");
		}

		TSharedPtr<FJsonObject> VersionJson;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(VersionJsonContents);
		if (!FJsonSerializer::Deserialize(JsonReader, VersionJson) || !VersionJson.IsValid())
		{
			return TEXT("Unknown");
		}

		FString Version;
		return VersionJson->TryGetStringField(TEXT("version"), Version) ? Version : TEXT("Unknown");
	}
}

namespace
{
void RequestViewportRedraw()
{
	if (GEditor)
	{
		GEditor->RedrawAllViewports(false);
	}
}

void UpdateEditorModeActivation()
{
	if (!GEditor)
	{
		return;
	}

	FEditorModeTools& ModeTools = GLevelEditorModeTools();
	const bool bShouldBeActive = GIsTrackingModeEnabled || GHasReverseMarker;
	if (bShouldBeActive)
	{
		if (!ModeTools.IsModeActive(FLandscapeHeightmapTrackerModule::EditorModeId))
		{
			ModeTools.ActivateMode(FLandscapeHeightmapTrackerModule::EditorModeId);
		}
	}
	else if (ModeTools.IsModeActive(FLandscapeHeightmapTrackerModule::EditorModeId))
	{
		ModeTools.DeactivateMode(FLandscapeHeightmapTrackerModule::EditorModeId);
	}
}
}

FLandscapeHeightmapTrackerModule::FOnViewportClickResult& FLandscapeHeightmapTrackerModule::OnViewportClickResult()
{
	return GOnViewportClickResult;
}

FLandscapeHeightmapTrackerModule::FOnViewportHoverResult& FLandscapeHeightmapTrackerModule::OnViewportHoverResult()
{
	return GOnViewportHoverResult;
}

void FLandscapeHeightmapTrackerModule::SetTrackingModeEnabled(bool bEnabled)
{
	GIsTrackingModeEnabled = bEnabled;
	UpdateEditorModeActivation();
}

bool FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled()
{
	return GIsTrackingModeEnabled;
}

FString FLandscapeHeightmapTrackerModule::GetPluginVersion()
{
	return ReadPluginVersion();
}

void FLandscapeHeightmapTrackerModule::SetReverseMarker(const FVector& WorldPosition, AActor* OwnerActor)
{
	GReverseMarkerWorldPosition = WorldPosition;
	GReverseMarkerOwner = OwnerActor;
	GHasReverseMarker = true;
	GReverseMarkerNeedsCleanup = false;
	UpdateEditorModeActivation();
	RequestViewportRedraw();
}

void FLandscapeHeightmapTrackerModule::ClearReverseMarker()
{
	GHasReverseMarker = false;
	GReverseMarkerNeedsCleanup = false;
	GReverseMarkerWorldPosition = FVector::ZeroVector;
	GReverseMarkerOwner.Reset();
	UpdateEditorModeActivation();
	RequestViewportRedraw();
}

bool FLandscapeHeightmapTrackerModule::GetReverseMarker(FVector& OutWorldPosition)
{
	if (!GHasReverseMarker)
	{
		return false;
	}

	if (!GReverseMarkerOwner.IsValid())
	{
		GHasReverseMarker = false;
		GReverseMarkerNeedsCleanup = true;
		return false;
	}

	OutWorldPosition = GReverseMarkerWorldPosition;
	return true;
}

bool FLandscapeHeightmapTrackerModule::ConsumeReverseMarkerCleanupRequest()
{
	if (!GReverseMarkerNeedsCleanup)
	{
		return false;
	}

	GReverseMarkerNeedsCleanup = false;
	return true;
}

void FLandscapeHeightmapTrackerModule::StartupModule()
{
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("LandscapeHeightmapTracker startup."));

	FLandscapeHeightmapTrackerStyle::Initialize();
	FLandscapeHeightmapTrackerCommands::Register();

	PluginCommands = MakeShared<FUICommandList>();
	PluginCommands->MapAction(
		FLandscapeHeightmapTrackerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::PluginButtonClicked),
		FCanExecuteAction());

	FEditorModeRegistry::Get().RegisterMode<FLandscapeHeightmapTrackerEdMode>(
		EditorModeId,
		LOCTEXT("LandscapeHeightmapTrackerEdModeName", "Landscape Heightmap Tracker"),
		FSlateIcon(),
		false);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLandscapeHeightmapTrackerModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PluginTabName, FOnSpawnTab::CreateRaw(this, &FLandscapeHeightmapTrackerModule::OnSpawnPluginTab))
		.SetDisplayName(FText::Format(LOCTEXT("LandscapeHeightmapTrackerTabTitle", "Landscape Heightmap Tracker {0}"), FText::FromString(GetPluginVersion())))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PaintLayersTabName, FOnSpawnTab::CreateRaw(this, &FLandscapeHeightmapTrackerModule::OnSpawnPaintLayersTab))
		.SetDisplayName(LOCTEXT("LandscapePaintLayersTabTitle", "Landscape Paint Layers"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLandscapeHeightmapTrackerModule::ShutdownModule()
{
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("LandscapeHeightmapTracker shutdown."));

	SetTrackingModeEnabled(false);
	ClearReverseMarker();
	GOnViewportClickResult.Clear();
	GOnViewportHoverResult.Clear();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PluginTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PaintLayersTabName);
	FEditorModeRegistry::Get().UnregisterMode(EditorModeId);

	FLandscapeHeightmapTrackerCommands::Unregister();
	FLandscapeHeightmapTrackerStyle::Shutdown();
}

TSharedRef<SDockTab> FLandscapeHeightmapTrackerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("Opening Landscape Heightmap Tracker tab."));
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLandscapeHeightmapTrackerPanel)
		];
}

TSharedRef<SDockTab> FLandscapeHeightmapTrackerModule::OnSpawnPaintLayersTab(const FSpawnTabArgs& SpawnTabArgs)
{
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("Opening Landscape Paint Layers tab."));
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLandscapePaintLayerBulkRemoveWidget)
		];
}

void FLandscapeHeightmapTrackerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(PluginTabName);
}

void FLandscapeHeightmapTrackerModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->FindOrAddSection("LandscapeHeightmapTracker");
	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().OpenPluginWindow, PluginCommands);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLandscapeHeightmapTrackerModule, LandscapeHeightmapTracker)
