#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture.h"

class UMaterialInstanceConstant;
class UStaticMesh;
class UTexture2D;

namespace LandscapeHeightmapTracker::ScanVault
{
enum class ETextureRole : uint8
{
	BaseColor,
	Normal,
	Roughness,
	AO,
	Displacement,
	Opacity,
	Specular,
	Translucency,
	Other,
	Invalid
};

enum class EImportIssueSeverity : uint8
{
	Info,
	Warning,
	Error
};

enum class EImportStatus : uint8
{
	Success,
	SuccessWithWarnings,
	Failed,
	Cancelled,
	PartialImport
};

enum class EConflictPolicy : uint8
{
	Cancel,
	UniqueSuffix
};

struct FImportIssue
{
	EImportIssueSeverity Severity = EImportIssueSeverity::Info;
	FString Code;
	FString Message;
};

struct FScanVaultSource
{
	FString AssetId;
	FString Name;
	FString AssetType;
	FString JsonPath;
	FString AssetFolderPath;
};

struct FScanVaultDestination
{
	FString BaseContentPath;
	FString ContentPath;
	FString AssetBaseName;
	FString OriginalAssetName;
};

struct FScanVaultMeshLod
{
	FString Variant;
	int32 Lod = INDEX_NONE;
	FString SourcePath;
	FString Format;
};

struct FScanVaultMesh
{
	bool bHasMesh = false;
	FString PrimaryVariant;
	TArray<FScanVaultMeshLod> Lods;
};

struct FScanVaultTexture
{
	ETextureRole Role = ETextureRole::Invalid;
	FString RoleText;
	FString SourcePath;
	FString MapType;
	FString SetKind;
	int32 Resolution = 0;
	FString Format;
};

struct FScanVaultTextureParameterMapping
{
	ETextureRole Role = ETextureRole::Invalid;
	FString RoleText;
	FName ParameterName;
};

struct FScanVaultMaterial
{
	FString ProfileId;
	FString ProfileName;
	TArray<FString> CompatibleAssetTypes;
	FString MasterMaterialPath;
	FString MaterialInstancePrefix;
	FString MaterialInstanceName;
	TArray<FScanVaultTextureParameterMapping> TextureParameterMappings;
};

struct FScanVaultOptions
{
	bool bImportLods = true;
	bool bEnableNanite = true;
	bool bCreateMaterialInstance = true;
	bool bReadinessOverride = false;
};

struct FScanVaultValidationState
{
	TArray<FString> Errors;
	TArray<FString> Warnings;
	TArray<FString> Infos;
};

struct FScanVaultManifest
{
	int32 SchemaVersion = 0;
	FString PackageId;
	FScanVaultSource Source;
	FScanVaultDestination Destination;
	FScanVaultMesh Mesh;
	TArray<FScanVaultTexture> Textures;
	FScanVaultMaterial Material;
	FScanVaultOptions Options;
	FScanVaultValidationState Validation;
};

struct FScanVaultImportPlan
{
	FString ManifestPath;
	FScanVaultManifest Manifest;
	TArray<FImportIssue> Issues;
	bool bCanImport = false;
};

struct FScanVaultCreatedAssets
{
	TMap<ETextureRole, TObjectPtr<UTexture2D>> TexturesByRole;
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;
	TObjectPtr<UMaterialInstanceConstant> MaterialInstance = nullptr;
	TArray<TObjectPtr<UObject>> SessionCreatedAssets;
	TArray<FString> SavedPackageNames;
};

struct FScanVaultImportReport
{
	EImportStatus Status = EImportStatus::Failed;
	FString PackageId;
	FString DestinationPath;
	int32 TexturesImported = 0;
	bool bMeshImported = false;
	int32 LodsImported = 0;
	bool bMaterialInstanceCreated = false;
	bool bNaniteEnabled = false;
	bool bAssetsSaved = false;
	double DurationSeconds = 0.0;
	TArray<FImportIssue> Issues;
	TArray<FString> ImportedObjectPaths;
	TArray<FString> SavedPackageNames;
	TArray<FString> LeftoverObjectPaths;
};

struct FScanVaultImportRequest
{
	FScanVaultImportPlan Plan;
	EConflictPolicy ConflictPolicy = EConflictPolicy::Cancel;
};

ETextureRole TextureRoleFromString(const FString& Value);
FString TextureRoleToString(ETextureRole Role);
FString TextureRoleShortSuffix(ETextureRole Role);
FString MakeTextureAssetName(const FString& AssetBaseName, ETextureRole Role);
FString MakeStaticMeshAssetName(const FString& AssetBaseName);
FString MakeMaterialInstanceAssetName(const FScanVaultManifest& Manifest);
bool IsValidContentPath(const FString& Path);
bool IsSupportedTextureExtension(const FString& Path);
bool IsSupportedMeshExtension(const FString& Path);
void ApplyTextureSettings(UTexture2D& Texture, ETextureRole Role);
void AddIssue(TArray<FImportIssue>& Issues, EImportIssueSeverity Severity, const FString& Code, const FString& Message);
bool HasBlockingIssues(const TArray<FImportIssue>& Issues);
FString ImportStatusToString(EImportStatus Status);
FString BuildReportText(const FScanVaultImportReport& Report);
void ValidateManifestForImport(const FScanVaultManifest& Manifest, TArray<FImportIssue>& Issues);
}
