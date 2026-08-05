#include "LandscapePaintLayerService.h"

#include "Editor.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "ScopedTransaction.h"
#include "Selection.h"

#define LOCTEXT_NAMESPACE "LandscapePaintLayerService"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapePaintLayerService, Log, All);

namespace
{
FString GetBlendType(const ULandscapeLayerInfoObject* LayerInfo)
{
	if (!LayerInfo)
	{
		return TEXT("Unknown");
	}

	switch (LayerInfo->GetBlendMethod())
	{
	case ELandscapeTargetLayerBlendMethod::None:
		return TEXT("No Weight Blending");
	case ELandscapeTargetLayerBlendMethod::FinalWeightBlending:
		return TEXT("Weight-Blended");
	case ELandscapeTargetLayerBlendMethod::PremultipliedAlphaBlending:
		return TEXT("Advanced Weight Blending");
	default:
		return TEXT("Unknown");
	}
}
}

ALandscapeProxy* FLandscapePaintLayerService::GetExactlyOneSelectedLandscape(FString& OutError)
{
	OutError.Reset();
	if (!GEditor)
	{
		OutError = TEXT("Editor selection is unavailable.");
		return nullptr;
	}

	ALandscapeProxy* SelectedLandscape = nullptr;
	int32 SelectedLandscapeCount = 0;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(*It))
		{
			SelectedLandscape = Landscape;
			++SelectedLandscapeCount;
		}
	}

	if (SelectedLandscapeCount != 1)
	{
		OutError = SelectedLandscapeCount == 0
			? TEXT("Select exactly one Landscape actor.")
			: TEXT("Multiple Landscape actors are selected. Select exactly one.");
		return nullptr;
	}

	return SelectedLandscape;
}

bool FLandscapePaintLayerService::GetTargetLayers(
	ALandscapeProxy* Landscape,
	TArray<FLandscapePaintTargetLayerInfo>& OutLayers,
	FString& OutError)
{
	OutLayers.Reset();
	OutError.Reset();
	if (!IsValid(Landscape))
	{
		OutError = TEXT("Landscape is invalid.");
		return false;
	}

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!IsValid(LandscapeInfo) || !LandscapeInfo->LandscapeActor.IsValid())
	{
		OutError = TEXT("The selected Landscape has no loaded main Landscape actor.");
		return false;
	}

	TSet<FName> MaterialLayerNames;
	MaterialLayerNames.Append(Landscape->RetrieveTargetLayerNamesFromMaterials());
	OutLayers.Reserve(LandscapeInfo->Layers.Num());
	for (const FLandscapeInfoLayerSettings& LayerSettings : LandscapeInfo->Layers)
	{
		const FName LayerName = LayerSettings.GetLayerName();
		if (LayerName.IsNone())
		{
			continue;
		}

		FLandscapePaintTargetLayerInfo& Layer = OutLayers.AddDefaulted_GetRef();
		Layer.LayerName = LayerName;
		Layer.LayerInfo = LayerSettings.LayerInfoObj;
		Layer.LayerInfoPath = Layer.LayerInfo ? Layer.LayerInfo->GetPathName() : TEXT("Unassigned");
		Layer.BlendType = GetBlendType(Layer.LayerInfo);
		Layer.bExistsInLandscapeMaterial = MaterialLayerNames.Contains(LayerName);
		Layer.bIsOrphaned = Layer.LayerInfo != nullptr && !Layer.bExistsInLandscapeMaterial;
	}

	OutLayers.Sort([](const FLandscapePaintTargetLayerInfo& A, const FLandscapePaintTargetLayerInfo& B)
	{
		return A.LayerName.LexicalLess(B.LayerName);
	});
	return true;
}

bool FLandscapePaintLayerService::ValidateLayerNames(
	const TArray<FLandscapePaintTargetLayerInfo>& AvailableLayers,
	const TArray<FName>& RequestedLayerNames,
	TArray<FName>& OutValidatedLayerNames,
	FString& OutError)
{
	OutValidatedLayerNames.Reset();
	OutError.Reset();
	if (RequestedLayerNames.IsEmpty())
	{
		OutError = TEXT("No Paint Target Layers were requested.");
		return false;
	}

	TSet<FName> AvailableNames;
	for (const FLandscapePaintTargetLayerInfo& Layer : AvailableLayers)
	{
		AvailableNames.Add(Layer.LayerName);
	}

	TSet<FName> SeenNames;
	for (const FName LayerName : RequestedLayerNames)
	{
		if (LayerName.IsNone())
		{
			OutError = TEXT("A requested layer has an empty name.");
			return false;
		}
		if (SeenNames.Contains(LayerName))
		{
			OutError = FString::Printf(TEXT("Layer '%s' was requested more than once."), *LayerName.ToString());
			return false;
		}
		if (!AvailableNames.Contains(LayerName))
		{
			OutError = FString::Printf(TEXT("Layer '%s' does not exist on the selected Landscape."), *LayerName.ToString());
			return false;
		}

		SeenNames.Add(LayerName);
		OutValidatedLayerNames.Add(LayerName);
	}

	return true;
}

FLandscapePaintLayerRemoveResult FLandscapePaintLayerService::RemoveTargetLayers(
	ALandscapeProxy* Landscape,
	const TArray<FName>& LayerNames)
{
	FLandscapePaintLayerRemoveResult Result;
	TArray<FLandscapePaintTargetLayerInfo> AvailableLayers;
	FString Error;
	if (!GetTargetLayers(Landscape, AvailableLayers, Error))
	{
		Result.Message = Error;
		return Result;
	}

	TArray<FName> ValidatedNames;
	if (!ValidateLayerNames(AvailableLayers, LayerNames, ValidatedNames, Error))
	{
		Result.Message = Error;
		return Result;
	}

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	ALandscape* MainLandscape = LandscapeInfo ? LandscapeInfo->LandscapeActor.Get() : nullptr;
	if (!MainLandscape)
	{
		Result.Message = TEXT("The main Landscape actor is not loaded.");
		return Result;
	}

	TMap<FName, TObjectPtr<ULandscapeLayerInfoObject>> LayerInfos;
	for (const FLandscapeInfoLayerSettings& LayerSettings : LandscapeInfo->Layers)
	{
		LayerInfos.Add(LayerSettings.GetLayerName(), LayerSettings.LayerInfoObj);
	}

	const FScopedTransaction Transaction(
		ValidatedNames.Num() == 1
			? LOCTEXT("RemovePaintTargetLayer", "Remove Landscape Paint Target Layer")
			: LOCTEXT("RemovePaintTargetLayers", "Remove Landscape Paint Target Layers"));
	FScopedSetLandscapeEditingLayer EditingLayerScope(
		MainLandscape,
		MainLandscape->GetEditingLayer(),
		[MainLandscape]()
		{
			MainLandscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_All);
		});

	for (const FName LayerName : ValidatedNames)
	{
		LandscapeInfo->DeleteLayer(LayerInfos.FindRef(LayerName), LayerName);
	}

	if (GEditor)
	{
		GEditor->RedrawLevelEditingViewports(true);
	}

	Result.bSuccess = true;
	Result.RemovedLayers = MoveTemp(ValidatedNames);
	Result.RemovedCount = Result.RemovedLayers.Num();
	Result.Message = FString::Printf(TEXT("Removed %d Landscape Paint Target Layer(s)."), Result.RemovedCount);
	UE_LOG(LogLandscapePaintLayerService, Log, TEXT("%s"), *Result.Message);
	return Result;
}

#undef LOCTEXT_NAMESPACE
