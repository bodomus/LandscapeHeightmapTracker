#include "LandscapeHeightmapTrackerModule.h"

#include "EditorModeManager.h"
#include "EditorModeRegistry.h"
#include "LandscapeHeightmapTrackerCommands.h"
#include "LandscapeHeightmapTrackerEdMode.h"
#include "LandscapeHeightmapTrackerStyle.h"
#include "LevelEditor.h"
#include "SLandscapeHeightmapTrackerPanel.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FLandscapeHeightmapTrackerModule"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeHeightmapTracker, Log, All);

const FName FLandscapeHeightmapTrackerModule::PluginTabName(TEXT("LandscapeHeightmapTracker"));
const FName FLandscapeHeightmapTrackerModule::EditorModeId(TEXT("EM_LandscapeHeightmapTracker"));

static FLandscapeHeightmapTrackerModule::FOnViewportClickResult GOnViewportClickResult;

FLandscapeHeightmapTrackerModule::FOnViewportClickResult& FLandscapeHeightmapTrackerModule::OnViewportClickResult()
{
	return GOnViewportClickResult;
}

void FLandscapeHeightmapTrackerModule::SetTrackingModeEnabled(bool bEnabled)
{
	if (!GEditor)
	{
		return;
	}

	FEditorModeTools& ModeTools = GLevelEditorModeTools();
	if (bEnabled)
	{
		if (!ModeTools.IsModeActive(EditorModeId))
		{
			ModeTools.ActivateMode(EditorModeId);
		}
	}
	else if (ModeTools.IsModeActive(EditorModeId))
	{
		ModeTools.DeactivateMode(EditorModeId);
	}
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
		.SetDisplayName(LOCTEXT("LandscapeHeightmapTrackerTabTitle", "Landscape Heightmap Tracker"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FLandscapeHeightmapTrackerModule::ShutdownModule()
{
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("LandscapeHeightmapTracker shutdown."));

	SetTrackingModeEnabled(false);
	GOnViewportClickResult.Clear();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PluginTabName);
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
