#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "UObject/WeakObjectPtr.h"

class FLandscapeHeightmapTrackerModule : public IModuleInterface
{
public:
	struct FViewportClickResult
	{
		FVector WorldPosition = FVector::ZeroVector;
		TWeakObjectPtr<AActor> HitActor;
		TWeakObjectPtr<UPrimitiveComponent> HitComponent;
	};

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnViewportClickResult, const FViewportClickResult&);

	static const FName PluginTabName;
	static const FName EditorModeId;

	static FOnViewportClickResult& OnViewportClickResult();
	static void SetTrackingModeEnabled(bool bEnabled);
	static bool IsTrackingModeEnabled();
	static void SetReverseMarker(const FVector& WorldPosition, AActor* OwnerActor);
	static void ClearReverseMarker();
	static bool GetReverseMarker(FVector& OutWorldPosition);
	static bool ConsumeReverseMarkerCleanupRequest();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void PluginButtonClicked();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<class FUICommandList> PluginCommands;
};
