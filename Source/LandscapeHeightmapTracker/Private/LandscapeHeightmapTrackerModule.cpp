#include "LandscapeHeightmapTrackerModule.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserItemPath.h"
#include "ContentBrowserModule.h"
#include "ContentRefreshPathUtils.h"
#include "EditorModeManager.h"
#include "EditorModeRegistry.h"
#include "Dom/JsonObject.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IContentBrowserSingleton.h"
#include "Interfaces/IPluginManager.h"
#include "LandscapeHeightmapTrackerCommands.h"
#include "LandscapeHeightmapTrackerEdMode.h"
#include "LandscapeHeightmapTrackerStyle.h"
#include "LevelEditor.h"
#include "ScanVault/ScanVaultImporter.h"
#include "ScanVault/ScanVaultManifestReader.h"
#include "ScanVault/SScanVaultImportWindow.h"
#include "SLandscapePaintLayerBulkRemoveWidget.h"
#include "SLandscapeHeightmapTrackerPanel.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"

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
	TSharedPtr<SNotificationItem> BeginRefreshNotification(const FString& VirtualPath)
	{
		FNotificationInfo NotificationInfo(FText::Format(
			LOCTEXT("RefreshingContentNotification", "Refreshing Content ({0})..."),
			FText::FromString(VirtualPath)));
		NotificationInfo.bFireAndForget = false;
		NotificationInfo.bUseThrobber = true;
		NotificationInfo.FadeOutDuration = 0.2f;
		return FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}

	void CompleteRefreshNotification(
		const TSharedPtr<SNotificationItem>& Notification,
		const FText& Text,
		SNotificationItem::ECompletionState CompletionState)
	{
		if (Notification.IsValid())
		{
			Notification->SetText(Text);
			Notification->SetCompletionState(CompletionState);
			Notification->ExpireAndFadeout();
		}
	}

	void ShowRefreshFailure(const FText& Text)
	{
		FNotificationInfo NotificationInfo(Text);
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.FadeOutDuration = 0.2f;
		const TSharedPtr<SNotificationItem> Notification =
			FSlateNotificationManager::Get().AddNotification(NotificationInfo);
		if (Notification.IsValid())
		{
			Notification->SetCompletionState(SNotificationItem::CS_Fail);
		}
	}

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
	PluginCommands->MapAction(
		FLandscapeHeightmapTrackerCommands::Get().RefreshContent,
		FExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::ExecuteRefreshContent),
		FCanExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::CanExecuteRefreshContent));
	PluginCommands->MapAction(
		FLandscapeHeightmapTrackerCommands::Get().RefreshCurrentFolder,
		FExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::ExecuteRefreshCurrentFolder),
		FCanExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::CanExecuteRefreshCurrentFolder));
	PluginCommands->MapAction(
		FLandscapeHeightmapTrackerCommands::Get().ScanVaultImport,
		FExecuteAction::CreateRaw(this, &FLandscapeHeightmapTrackerModule::ExecuteScanVaultImport),
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

void FLandscapeHeightmapTrackerModule::ExecuteRefreshContent()
{
	RefreshContentPath(TEXT("/Game"));
}

void FLandscapeHeightmapTrackerModule::ExecuteRefreshCurrentFolder()
{
	FContentBrowserModule* ContentBrowserModule =
		FModuleManager::Get().LoadModulePtr<FContentBrowserModule>(TEXT("ContentBrowser"));
	if (!ContentBrowserModule)
	{
		UE_LOG(LogLandscapeHeightmapTracker, Error, TEXT("Content refresh failed: ContentBrowser module is unavailable."));
		ShowRefreshFailure(LOCTEXT("RefreshCurrentFolderNoContentBrowser", "Content refresh failed. See Output Log."));
		return;
	}

	const FContentBrowserItemPath CurrentPath = ContentBrowserModule->Get().GetCurrentPath();
	if (!CurrentPath.HasInternalPath())
	{
		UE_LOG(LogLandscapeHeightmapTracker, Warning, TEXT("Content refresh skipped: the current Content Browser path has no internal package path."));
		ShowRefreshFailure(LOCTEXT("RefreshCurrentFolderNoInternalPath", "Select a folder under /Game and try again."));
		return;
	}

	const FString InternalPath = CurrentPath.GetInternalPathString();
	if (!LandscapeHeightmapTracker::ContentRefresh::IsProjectContentPath(InternalPath))
	{
		UE_LOG(LogLandscapeHeightmapTracker, Warning, TEXT("Content refresh rejected non-project path '%s'."), *InternalPath);
		ShowRefreshFailure(LOCTEXT("RefreshCurrentFolderOutsideGame", "Refresh Current Folder is limited to /Game."));
		return;
	}

	RefreshContentPath(InternalPath);
}

void FLandscapeHeightmapTrackerModule::ExecuteScanVaultImport()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ScanVaultNoDesktopPlatform", "ScanVault import failed. Desktop platform service is unavailable."));
		return;
	}

	TArray<FString> SelectedFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bPicked = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("ScanVault Import Package"),
		FString(),
		FString(),
		TEXT("ScanVault manifests (*.scanvault-ue.json)|*.scanvault-ue.json|JSON files (*.json)|*.json"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (!bPicked || SelectedFiles.IsEmpty())
	{
		return;
	}

	LandscapeHeightmapTracker::ScanVault::FScanVaultImportPlan ImportPlan =
		LandscapeHeightmapTracker::ScanVault::FScanVaultManifestReader::ReadFromFile(SelectedFiles[0]);

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("ScanVaultImportWindowTitle", "ScanVault Import Package"))
		.ClientSize(FVector2D(720.0f, 620.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	Window->SetContent(
		SNew(LandscapeHeightmapTracker::ScanVault::SScanVaultImportWindow)
		.OwnerWindow(Window)
		.ImportPlan(ImportPlan)
		.OnImportConfirmed(LandscapeHeightmapTracker::ScanVault::FOnScanVaultImportConfirmed::CreateLambda(
			[](const LandscapeHeightmapTracker::ScanVault::FScanVaultImportPlan& ConfirmedPlan, LandscapeHeightmapTracker::ScanVault::EConflictPolicy ConflictPolicy)
			{
				LandscapeHeightmapTracker::ScanVault::FScanVaultImportRequest Request;
				Request.Plan = ConfirmedPlan;
				Request.ConflictPolicy = ConflictPolicy;
				const LandscapeHeightmapTracker::ScanVault::FScanVaultImportReport Report =
					LandscapeHeightmapTracker::ScanVault::FScanVaultImporter::Import(Request);
				const FString ReportText = LandscapeHeightmapTracker::ScanVault::BuildReportText(Report);
				FPlatformApplicationMisc::ClipboardCopy(*ReportText);
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ReportText));
			})));

	FSlateApplication::Get().AddModalWindow(Window, nullptr);
}

void FLandscapeHeightmapTrackerModule::RefreshContentPath(const FString& VirtualPath)
{
	if (bIsRefreshingContent)
	{
		UE_LOG(LogLandscapeHeightmapTracker, Verbose, TEXT("Ignoring overlapping content refresh request for '%s'."), *VirtualPath);
		return;
	}

	if (!LandscapeHeightmapTracker::ContentRefresh::IsProjectContentPath(VirtualPath))
	{
		UE_LOG(LogLandscapeHeightmapTracker, Error, TEXT("Content refresh rejected invalid project path '%s'."), *VirtualPath);
		ShowRefreshFailure(LOCTEXT("RefreshContentInvalidPath", "Content refresh failed. See Output Log."));
		return;
	}

	TGuardValue<bool> RefreshGuard(bIsRefreshingContent, true);
	const double StartTimeSeconds = FPlatformTime::Seconds();
	const TSharedPtr<SNotificationItem> Notification = BeginRefreshNotification(VirtualPath);
	UE_LOG(LogLandscapeHeightmapTracker, Log, TEXT("Starting forced Asset Registry scan for '%s'."), *VirtualPath);

	FAssetRegistryModule* AssetRegistryModule =
		FModuleManager::Get().LoadModulePtr<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if (!AssetRegistryModule)
	{
		UE_LOG(LogLandscapeHeightmapTracker, Error, TEXT("Content refresh failed: AssetRegistry module is unavailable."));
		CompleteRefreshNotification(
			Notification,
			LOCTEXT("RefreshContentAssetRegistryUnavailable", "Content refresh failed. See Output Log."),
			SNotificationItem::CS_Fail);
		return;
	}

	TArray<FString> PathsToScan;
	PathsToScan.Add(VirtualPath);
	AssetRegistryModule->Get().ScanPathsSynchronous(
		PathsToScan,
		/* bForceRescan */ true,
		/* bIgnoreDenyListScanFilters */ false);

	const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
	UE_LOG(
		LogLandscapeHeightmapTracker,
		Log,
		TEXT("Forced Asset Registry scan for '%s' completed in %.2f seconds."),
		*VirtualPath,
		ElapsedSeconds);
	CompleteRefreshNotification(
		Notification,
		FText::Format(
			LOCTEXT("RefreshContentCompleted", "Content refresh completed in {0} s."),
			FText::AsNumber(ElapsedSeconds, &FNumberFormattingOptions().SetMaximumFractionalDigits(2))),
		SNotificationItem::CS_Success);
}

bool FLandscapeHeightmapTrackerModule::CanExecuteRefreshContent() const
{
	return !bIsRefreshingContent;
}

bool FLandscapeHeightmapTrackerModule::CanExecuteRefreshCurrentFolder() const
{
	if (bIsRefreshingContent)
	{
		return false;
	}

	const FContentBrowserModule* ContentBrowserModule =
		FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser"));
	if (!ContentBrowserModule)
	{
		return false;
	}

	const FContentBrowserItemPath CurrentPath = ContentBrowserModule->Get().GetCurrentPath();
	return CurrentPath.HasInternalPath()
		&& LandscapeHeightmapTracker::ContentRefresh::IsProjectContentPath(CurrentPath.GetInternalPathString());
}

void FLandscapeHeightmapTrackerModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->FindOrAddSection("LandscapeHeightmapTracker");
	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().OpenPluginWindow, PluginCommands);
	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().RefreshContent, PluginCommands);
	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().RefreshCurrentFolder, PluginCommands);
	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().ScanVaultImport, PluginCommands);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLandscapeHeightmapTrackerModule, LandscapeHeightmapTracker)
