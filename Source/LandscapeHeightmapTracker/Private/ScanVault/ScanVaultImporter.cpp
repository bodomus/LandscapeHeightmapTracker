#include "ScanVault/ScanVaultImporter.h"

#include "AssetImportTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxStaticMeshImportData.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "IAssetTools.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#include "ObjectTools.h"
#include "StaticMeshEditorSubsystem.h"
#include "UObject/SavePackage.h"

namespace LandscapeHeightmapTracker::ScanVault
{
namespace
{
DEFINE_LOG_CATEGORY_STATIC(LogScanVaultImporter, Log, All);

FString ObjectPathFor(const UObject* Object)
{
	return Object ? Object->GetPathName() : FString();
}

FString ResolveAssetName(const FString& PackagePath, const FString& DesiredName, EConflictPolicy ConflictPolicy, bool& bOutConflict)
{
	bOutConflict = false;
	const FString DesiredPackageName = PackagePath / DesiredName;
	const FString DesiredObjectPath = PackagePath / DesiredName + TEXT(".") + DesiredName;
	if (!StaticFindObject(UObject::StaticClass(), nullptr, *DesiredObjectPath) && !FPackageName::DoesPackageExist(DesiredPackageName))
	{
		return DesiredName;
	}

	bOutConflict = true;
	if (ConflictPolicy == EConflictPolicy::Cancel)
	{
		return FString();
	}

	FString UniquePackageName;
	FString UniqueAssetName;
	IAssetTools::Get().CreateUniqueAssetName(PackagePath / DesiredName, TEXT(""), UniquePackageName, UniqueAssetName);
	return UniqueAssetName;
}

UObject* ImportSingleAsset(
	const FString& SourcePath,
	const FString& PackagePath,
	const FString& DesiredName,
	EConflictPolicy ConflictPolicy,
	UObject* Options,
	TArray<FImportIssue>& Issues,
	TArray<UObject*>& OutCreatedAssets)
{
	bool bHadConflict = false;
	const FString AssetName = ResolveAssetName(PackagePath, DesiredName, ConflictPolicy, bHadConflict);
	if (AssetName.IsEmpty())
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("asset.conflict"), FString::Printf(TEXT("Asset already exists and overwrite is disabled: %s/%s"), *PackagePath, *DesiredName));
		return nullptr;
	}
	if (bHadConflict)
	{
		AddIssue(Issues, EImportIssueSeverity::Warning, TEXT("asset.conflict.unique"), FString::Printf(TEXT("Asset name conflict for %s/%s. Imported as %s."), *PackagePath, *DesiredName, *AssetName));
	}

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->Filename = SourcePath;
	Task->DestinationPath = PackagePath;
	Task->DestinationName = AssetName;
	Task->bAutomated = true;
	Task->bAsync = false;
	Task->bSave = false;
	Task->bReplaceExisting = false;
	Task->bReplaceExistingSettings = false;
	Task->Options = Options;

	TArray<UAssetImportTask*> Tasks;
	Tasks.Add(Task);
	IAssetTools::Get().ImportAssetTasks(Tasks);

	const TArray<UObject*>& Objects = Task->GetObjects();
	if (Objects.IsEmpty() || !Objects[0])
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("asset.import.failed"), FString::Printf(TEXT("Failed to import source: %s"), *SourcePath));
		return nullptr;
	}

	UObject* Imported = Objects[0];
	OutCreatedAssets.Add(Imported);
	return Imported;
}

UFbxImportUI* MakeStaticMeshImportOptions(bool bEnableNanite)
{
	UFbxImportUI* ImportUI = NewObject<UFbxImportUI>();
	ImportUI->bAutomatedImportShouldDetectType = false;
	ImportUI->MeshTypeToImport = FBXIT_StaticMesh;
	ImportUI->OriginalImportType = FBXIT_StaticMesh;
	ImportUI->bImportAsSkeletal = false;
	ImportUI->bImportMesh = true;
	ImportUI->bImportMaterials = false;
	ImportUI->bImportTextures = false;
	ImportUI->bCreatePhysicsAsset = false;
	ImportUI->bAutoComputeLodDistances = true;

	if (ImportUI->StaticMeshImportData)
	{
		ImportUI->StaticMeshImportData->bBuildNanite = bEnableNanite;
		ImportUI->StaticMeshImportData->bGenerateLightmapUVs = true;
		ImportUI->StaticMeshImportData->bAutoGenerateCollision = true;
		ImportUI->StaticMeshImportData->bCombineMeshes = true;
	}

	return ImportUI;
}

const FScanVaultMeshLod* FindPrimaryLod(const FScanVaultManifest& Manifest)
{
	return Manifest.Mesh.Lods.FindByPredicate([](const FScanVaultMeshLod& Lod)
	{
		return Lod.Lod == 0;
	});
}

UStaticMeshEditorSubsystem* GetStaticMeshEditorSubsystem()
{
	if (!GEditor)
	{
		return nullptr;
	}

	return GEditor->GetEditorSubsystem<UStaticMeshEditorSubsystem>();
}

void AddImportedPath(const UObject* Object, FScanVaultImportReport& Report)
{
	const FString Path = ObjectPathFor(Object);
	if (!Path.IsEmpty())
	{
		Report.ImportedObjectPaths.AddUnique(Path);
	}
}

void ConfigureImportedTexture(UTexture2D* Texture, const FScanVaultTexture& TextureManifest, FScanVaultImportReport& Report)
{
	if (!Texture)
	{
		return;
	}

	ApplyTextureSettings(*Texture, TextureManifest.Role);
	if (TextureManifest.Role == ETextureRole::Roughness && TextureManifest.MapType.Equals(TEXT("gloss"), ESearchCase::IgnoreCase))
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("texture.gloss"), TEXT("Original map type is Gloss. No pixel inversion was performed."));
	}
}

UMaterialInterface* LoadMasterMaterial(const FScanVaultManifest& Manifest, FScanVaultImportReport& Report)
{
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *Manifest.Material.MasterMaterialPath);
	if (!Material)
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("material.master.not_found"), FString::Printf(TEXT("Master Material not found: %s"), *Manifest.Material.MasterMaterialPath));
	}
	return Material;
}

UMaterialInstanceConstant* CreateMaterialInstance(
	const FScanVaultManifest& Manifest,
	UMaterialInterface* Parent,
	EConflictPolicy ConflictPolicy,
	FScanVaultImportReport& Report,
	TArray<UObject*>& OutCreatedAssets)
{
	if (!Parent)
	{
		return nullptr;
	}

	const FString DesiredName = MakeMaterialInstanceAssetName(Manifest);
	bool bHadConflict = false;
	const FString AssetName = ResolveAssetName(Manifest.Destination.ContentPath, DesiredName, ConflictPolicy, bHadConflict);
	if (AssetName.IsEmpty())
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("material.mi.conflict"), FString::Printf(TEXT("Material Instance already exists and overwrite is disabled: %s/%s"), *Manifest.Destination.ContentPath, *DesiredName));
		return nullptr;
	}
	if (bHadConflict)
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("material.mi.unique"), FString::Printf(TEXT("Material Instance name conflict for %s. Created %s."), *DesiredName, *AssetName));
	}

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = Parent;
	UObject* Created = IAssetTools::Get().CreateAsset(
		AssetName,
		Manifest.Destination.ContentPath,
		UMaterialInstanceConstant::StaticClass(),
		Factory,
		TEXT("ScanVaultImporter"));

	UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Created);
	if (!Instance)
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("material.mi.create_failed"), TEXT("Failed to create Material Instance."));
		return nullptr;
	}

	OutCreatedAssets.Add(Instance);
	Report.bMaterialInstanceCreated = true;
	AddImportedPath(Instance, Report);
	return Instance;
}

bool ParentHasTextureParameter(UMaterialInterface* Material, FName ParameterName)
{
	if (!Material || ParameterName.IsNone())
	{
		return false;
	}

	TArray<FMaterialParameterInfo> ParameterInfos;
	TArray<FGuid> ParameterIds;
	Material->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
	return ParameterInfos.ContainsByPredicate([ParameterName](const FMaterialParameterInfo& Info)
	{
		return Info.Name == ParameterName;
	});
}

void AssignTextureParameters(
	const FScanVaultManifest& Manifest,
	UMaterialInterface* Parent,
	UMaterialInstanceConstant* Instance,
	const TMap<ETextureRole, UTexture2D*>& TexturesByRole,
	FScanVaultImportReport& Report)
{
	if (!Instance)
	{
		return;
	}

	for (const FScanVaultTextureParameterMapping& Mapping : Manifest.Material.TextureParameterMappings)
	{
		if (Mapping.Role == ETextureRole::Invalid || Mapping.ParameterName.IsNone())
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("material.mapping.invalid"), FString::Printf(TEXT("Invalid texture mapping role '%s'."), *Mapping.RoleText));
			continue;
		}

		UTexture2D* const* Texture = TexturesByRole.Find(Mapping.Role);
		if (!Texture || !*Texture)
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("material.mapping.texture_missing"), FString::Printf(TEXT("No imported texture for role %s."), *TextureRoleToString(Mapping.Role)));
			continue;
		}

		if (!ParentHasTextureParameter(Parent, Mapping.ParameterName))
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("material.mapping.parameter_missing"), FString::Printf(TEXT("Master Material has no texture parameter '%s'."), *Mapping.ParameterName.ToString()));
			continue;
		}

		UTexture* TextureValue = *Texture;
		if (!UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(Instance, Mapping.ParameterName, TextureValue))
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("material.mapping.assign_failed"), FString::Printf(TEXT("Failed to assign texture parameter '%s'."), *Mapping.ParameterName.ToString()));
		}
	}

	Instance->PostEditChange();
	Instance->MarkPackageDirty();
}

void AssignMaterialToMesh(UStaticMesh* Mesh, UMaterialInterface* Material, FScanVaultImportReport& Report)
{
	if (!Mesh || !Material)
	{
		return;
	}

	const int32 SlotCount = Mesh->GetStaticMaterials().Num();
	if (SlotCount == 1)
	{
		Mesh->SetMaterial(0, Material);
		Mesh->PostEditChange();
		Mesh->MarkPackageDirty();
	}
	else if (SlotCount > 1)
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("mesh.material.multislot"), FString::Printf(TEXT("Imported mesh has %d material slots. Material Instance was not assigned automatically."), SlotCount));
	}
	else
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("mesh.material.no_slots"), TEXT("Imported mesh has no material slots. Material Instance was not assigned."));
	}
}

void ApplyNanite(UStaticMesh* Mesh, const FScanVaultManifest& Manifest, FScanVaultImportReport& Report)
{
	if (!Mesh)
	{
		return;
	}

	if (!Manifest.Options.bEnableNanite)
	{
		Report.bNaniteEnabled = false;
		return;
	}

	FMeshNaniteSettings NaniteSettings = Mesh->GetNaniteSettings();
	NaniteSettings.bEnabled = true;
	Mesh->SetNaniteSettings(NaniteSettings);
	Mesh->Build(false);
	Mesh->PostEditChange();
	Mesh->MarkPackageDirty();
	Report.bNaniteEnabled = Mesh->GetNaniteSettings().bEnabled;
	if (!Report.bNaniteEnabled)
	{
		AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("mesh.nanite.failed"), TEXT("Nanite was requested but was not enabled on the imported mesh."));
	}
}

bool SaveSessionAssets(const TArray<UObject*>& Assets, FScanVaultImportReport& Report)
{
	TArray<UPackage*> Packages;
	for (UObject* Asset : Assets)
	{
		if (Asset && Asset->GetPackage())
		{
			Packages.AddUnique(Asset->GetPackage());
		}
	}

	bool bAllSaved = true;
	for (UPackage* Package : Packages)
	{
		const FString PackageName = Package->GetName();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		if (!UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs))
		{
			bAllSaved = false;
			AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("asset.save.failed"), FString::Printf(TEXT("Failed to save package: %s"), *PackageName));
		}
		else
		{
			Report.SavedPackageNames.Add(PackageName);
		}
	}

	Report.bAssetsSaved = bAllSaved;
	return bAllSaved;
}

bool CleanupCreatedAssets(const TArray<UObject*>& Assets, FScanVaultImportReport& Report)
{
	TArray<UObject*> AssetsToDelete;
	for (UObject* Asset : Assets)
	{
		if (Asset)
		{
			AssetsToDelete.Add(Asset);
		}
	}

	if (AssetsToDelete.IsEmpty())
	{
		return true;
	}

	const int32 DeletedCount = ObjectTools::DeleteObjectsUnchecked(AssetsToDelete);
	if (DeletedCount != AssetsToDelete.Num())
	{
		for (UObject* Asset : AssetsToDelete)
		{
			if (IsValid(Asset))
			{
				Report.LeftoverObjectPaths.Add(ObjectPathFor(Asset));
			}
		}
		return false;
	}

	return true;
}
}

FScanVaultImportReport FScanVaultImporter::Import(const FScanVaultImportRequest& Request)
{
	FScanVaultImportReport Report;
	const double StartTime = FPlatformTime::Seconds();
	const FScanVaultManifest& Manifest = Request.Plan.Manifest;
	Report.PackageId = Manifest.PackageId;
	Report.DestinationPath = Manifest.Destination.ContentPath;
	Report.Issues = Request.Plan.Issues;

	UE_LOG(
		LogScanVaultImporter,
		Log,
		TEXT("ScanVault import started. PackageId=%s DestinationPath=%s TextureCount=%d LodCount=%d CreateMaterialInstance=%s EnableNanite=%s ImportLods=%s"),
		*Manifest.PackageId,
		*Manifest.Destination.ContentPath,
		Manifest.Textures.Num(),
		Manifest.Mesh.Lods.Num(),
		Manifest.Options.bCreateMaterialInstance ? TEXT("true") : TEXT("false"),
		Manifest.Options.bEnableNanite ? TEXT("true") : TEXT("false"),
		Manifest.Options.bImportLods ? TEXT("true") : TEXT("false"));

	if (!Request.Plan.bCanImport || HasBlockingIssues(Report.Issues))
	{
		Report.Status = EImportStatus::Failed;
		Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
		return Report;
	}

	TArray<UObject*> SessionCreatedAssets;
	TMap<ETextureRole, UTexture2D*> TexturesByRole;

	for (const FScanVaultTexture& TextureManifest : Manifest.Textures)
	{
		const FString DesiredName = MakeTextureAssetName(Manifest.Destination.AssetBaseName, TextureManifest.Role);
		UObject* ImportedObject = ImportSingleAsset(
			TextureManifest.SourcePath,
			Manifest.Destination.ContentPath,
			DesiredName,
			Request.ConflictPolicy,
			nullptr,
			Report.Issues,
			SessionCreatedAssets);

		UTexture2D* ImportedTexture = Cast<UTexture2D>(ImportedObject);
		if (!ImportedTexture)
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("texture.import.type"), FString::Printf(TEXT("Imported object is not Texture2D for role %s."), *TextureRoleToString(TextureManifest.Role)));
			CleanupCreatedAssets(SessionCreatedAssets, Report);
			Report.Status = Report.LeftoverObjectPaths.IsEmpty() ? EImportStatus::Failed : EImportStatus::PartialImport;
			Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
			return Report;
		}

		ConfigureImportedTexture(ImportedTexture, TextureManifest, Report);
		TexturesByRole.Add(TextureManifest.Role, ImportedTexture);
		++Report.TexturesImported;
		AddImportedPath(ImportedTexture, Report);
	}

	UStaticMesh* ImportedMesh = nullptr;
	if (Manifest.Mesh.bHasMesh)
	{
		const FScanVaultMeshLod* PrimaryLod = FindPrimaryLod(Manifest);
		if (!PrimaryLod)
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("mesh.primary.missing"), TEXT("No declared LOD 0 primary mesh."));
			CleanupCreatedAssets(SessionCreatedAssets, Report);
			Report.Status = Report.LeftoverObjectPaths.IsEmpty() ? EImportStatus::Failed : EImportStatus::PartialImport;
			Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
			return Report;
		}

		UFbxImportUI* MeshOptions = MakeStaticMeshImportOptions(Manifest.Options.bEnableNanite);
		UObject* ImportedObject = ImportSingleAsset(
			PrimaryLod->SourcePath,
			Manifest.Destination.ContentPath,
			MakeStaticMeshAssetName(Manifest.Destination.AssetBaseName),
			Request.ConflictPolicy,
			MeshOptions,
			Report.Issues,
			SessionCreatedAssets);

		ImportedMesh = Cast<UStaticMesh>(ImportedObject);
		if (!ImportedMesh)
		{
			AddIssue(Report.Issues, EImportIssueSeverity::Error, TEXT("mesh.import.failed"), TEXT("Primary mesh import did not produce a StaticMesh."));
			CleanupCreatedAssets(SessionCreatedAssets, Report);
			Report.Status = Report.LeftoverObjectPaths.IsEmpty() ? EImportStatus::Failed : EImportStatus::PartialImport;
			Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
			return Report;
		}

		Report.bMeshImported = true;
		AddImportedPath(ImportedMesh, Report);

		if (Manifest.Options.bImportLods)
		{
			UStaticMeshEditorSubsystem* StaticMeshSubsystem = GetStaticMeshEditorSubsystem();
			if (!StaticMeshSubsystem)
			{
				AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("mesh.lod.subsystem_missing"), TEXT("StaticMeshEditorSubsystem is unavailable. LOD import skipped."));
			}
			else
			{
				for (const FScanVaultMeshLod& Lod : Manifest.Mesh.Lods)
				{
					if (Lod.Lod == 0)
					{
						continue;
					}

					const int32 ImportedLodIndex = StaticMeshSubsystem->ImportLOD(ImportedMesh, Lod.Lod, Lod.SourcePath);
					if (ImportedLodIndex == INDEX_NONE)
					{
						AddIssue(Report.Issues, EImportIssueSeverity::Warning, TEXT("mesh.lod.import_failed"), FString::Printf(TEXT("Failed to import LOD %d from %s."), Lod.Lod, *Lod.SourcePath));
					}
					else
					{
						++Report.LodsImported;
					}
				}
			}
		}
		else
		{
			const int32 Skipped = FMath::Max(0, Manifest.Mesh.Lods.Num() - 1);
			if (Skipped > 0)
			{
				AddIssue(Report.Issues, EImportIssueSeverity::Info, TEXT("mesh.lod.skipped"), FString::Printf(TEXT("Skipped %d LOD(s) because importLods is false."), Skipped));
			}
		}

		ApplyNanite(ImportedMesh, Manifest, Report);
	}

	UMaterialInstanceConstant* MaterialInstance = nullptr;
	if (Manifest.Options.bCreateMaterialInstance)
	{
		UMaterialInterface* ParentMaterial = LoadMasterMaterial(Manifest, Report);
		if (!ParentMaterial)
		{
			CleanupCreatedAssets(SessionCreatedAssets, Report);
			Report.Status = Report.LeftoverObjectPaths.IsEmpty() ? EImportStatus::Failed : EImportStatus::PartialImport;
			Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
			return Report;
		}

		MaterialInstance = CreateMaterialInstance(Manifest, ParentMaterial, Request.ConflictPolicy, Report, SessionCreatedAssets);
		if (!MaterialInstance)
		{
			CleanupCreatedAssets(SessionCreatedAssets, Report);
			Report.Status = Report.LeftoverObjectPaths.IsEmpty() ? EImportStatus::Failed : EImportStatus::PartialImport;
			Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
			return Report;
		}

		AssignTextureParameters(Manifest, ParentMaterial, MaterialInstance, TexturesByRole, Report);
		AssignMaterialToMesh(ImportedMesh, MaterialInstance, Report);
	}

	if (!SaveSessionAssets(SessionCreatedAssets, Report))
	{
		const bool bCleanupComplete = CleanupCreatedAssets(SessionCreatedAssets, Report);
		ApplyFatalFailureCleanupStatus(Report, bCleanupComplete);
		Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;
		return Report;
	}

	const bool bHasWarnings = Report.Issues.ContainsByPredicate([](const FImportIssue& Issue)
	{
		return Issue.Severity == EImportIssueSeverity::Warning;
	});
	Report.Status = bHasWarnings ? EImportStatus::SuccessWithWarnings : EImportStatus::Success;
	Report.DurationSeconds = FPlatformTime::Seconds() - StartTime;

	UE_LOG(
		LogScanVaultImporter,
		Log,
		TEXT("ScanVault import completed. PackageId=%s Result=%s DurationMs=%.0f"),
		*Manifest.PackageId,
		*ImportStatusToString(Report.Status),
		Report.DurationSeconds * 1000.0);

	return Report;
}
}
