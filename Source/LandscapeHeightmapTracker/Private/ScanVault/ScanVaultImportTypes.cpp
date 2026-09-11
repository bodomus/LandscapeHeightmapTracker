#include "ScanVault/ScanVaultImportTypes.h"

#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

namespace LandscapeHeightmapTracker::ScanVault
{
namespace
{
FString NormalizeToken(FString Value)
{
	Value.TrimStartAndEndInline();
	Value.ReplaceInline(TEXT("_"), TEXT(""));
	Value.ReplaceInline(TEXT("-"), TEXT(""));
	return Value.ToLower();
}

bool FileExistsAndReadable(const FString& Path)
{
	if (Path.IsEmpty() || IFileManager::Get().DirectoryExists(*Path))
	{
		return false;
	}

	return IFileManager::Get().FileSize(*Path) >= 0;
}

bool IsScanVaultBlockingValidation(const FScanVaultManifest& Manifest)
{
	return Manifest.Validation.Errors.Num() > 0;
}

bool HasReadinessOverride(const FScanVaultManifest& Manifest)
{
	return Manifest.Options.bReadinessOverride;
}
}

ETextureRole TextureRoleFromString(const FString& Value)
{
	const FString Normalized = NormalizeToken(Value);
	if (Normalized == TEXT("basecolor") || Normalized == TEXT("albedo"))
	{
		return ETextureRole::BaseColor;
	}
	if (Normalized == TEXT("normal"))
	{
		return ETextureRole::Normal;
	}
	if (Normalized == TEXT("roughness"))
	{
		return ETextureRole::Roughness;
	}
	if (Normalized == TEXT("ao") || Normalized == TEXT("ambientocclusion"))
	{
		return ETextureRole::AO;
	}
	if (Normalized == TEXT("displacement"))
	{
		return ETextureRole::Displacement;
	}
	if (Normalized == TEXT("opacity"))
	{
		return ETextureRole::Opacity;
	}
	if (Normalized == TEXT("specular"))
	{
		return ETextureRole::Specular;
	}
	if (Normalized == TEXT("translucency"))
	{
		return ETextureRole::Translucency;
	}
	if (Normalized == TEXT("other"))
	{
		return ETextureRole::Other;
	}
	return ETextureRole::Invalid;
}

FString TextureRoleToString(ETextureRole Role)
{
	switch (Role)
	{
	case ETextureRole::BaseColor:
		return TEXT("baseColor");
	case ETextureRole::Normal:
		return TEXT("normal");
	case ETextureRole::Roughness:
		return TEXT("roughness");
	case ETextureRole::AO:
		return TEXT("ao");
	case ETextureRole::Displacement:
		return TEXT("displacement");
	case ETextureRole::Opacity:
		return TEXT("opacity");
	case ETextureRole::Specular:
		return TEXT("specular");
	case ETextureRole::Translucency:
		return TEXT("translucency");
	case ETextureRole::Other:
		return TEXT("other");
	default:
		return TEXT("invalid");
	}
}

FString TextureRoleShortSuffix(ETextureRole Role)
{
	switch (Role)
	{
	case ETextureRole::BaseColor:
		return TEXT("BC");
	case ETextureRole::Normal:
		return TEXT("N");
	case ETextureRole::Roughness:
		return TEXT("R");
	case ETextureRole::AO:
		return TEXT("AO");
	case ETextureRole::Displacement:
		return TEXT("D");
	case ETextureRole::Opacity:
		return TEXT("O");
	case ETextureRole::Specular:
		return TEXT("S");
	case ETextureRole::Translucency:
		return TEXT("T");
	case ETextureRole::Other:
		return TEXT("Other");
	default:
		return TEXT("Invalid");
	}
}

FString MakeTextureAssetName(const FString& AssetBaseName, ETextureRole Role)
{
	return FString::Printf(TEXT("T_%s_%s"), *AssetBaseName, *TextureRoleShortSuffix(Role));
}

FString MakeStaticMeshAssetName(const FString& AssetBaseName)
{
	return FString::Printf(TEXT("SM_%s"), *AssetBaseName);
}

FString MakeMaterialInstanceAssetName(const FScanVaultManifest& Manifest)
{
	if (!Manifest.Material.MaterialInstanceName.IsEmpty())
	{
		return Manifest.Material.MaterialInstanceName;
	}

	const FString Prefix = Manifest.Material.MaterialInstancePrefix.IsEmpty()
		? TEXT("MI_")
		: Manifest.Material.MaterialInstancePrefix;
	return Prefix + Manifest.Destination.AssetBaseName;
}

bool IsValidContentPath(const FString& Path)
{
	return !Path.IsEmpty()
		&& Path.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)
		&& FPackageName::IsValidLongPackageName(Path);
}

bool IsSupportedTextureExtension(const FString& Path)
{
	const FString Extension = FPaths::GetExtension(Path, true).ToLower();
	return Extension == TEXT(".png")
		|| Extension == TEXT(".jpg")
		|| Extension == TEXT(".jpeg")
		|| Extension == TEXT(".tga")
		|| Extension == TEXT(".exr")
		|| Extension == TEXT(".bmp")
		|| Extension == TEXT(".tif")
		|| Extension == TEXT(".tiff");
}

bool IsSupportedMeshExtension(const FString& Path)
{
	const FString Extension = FPaths::GetExtension(Path, true).ToLower();
	return Extension == TEXT(".fbx") || Extension == TEXT(".obj") || Extension == TEXT(".abc");
}

void ApplyTextureSettings(UTexture2D& Texture, ETextureRole Role)
{
	switch (Role)
	{
	case ETextureRole::BaseColor:
		Texture.SRGB = true;
		break;
	case ETextureRole::Normal:
		Texture.SRGB = false;
		Texture.CompressionSettings = TC_Normalmap;
		break;
	case ETextureRole::Roughness:
	case ETextureRole::AO:
	case ETextureRole::Displacement:
	case ETextureRole::Opacity:
	case ETextureRole::Specular:
	case ETextureRole::Translucency:
	case ETextureRole::Other:
		Texture.SRGB = false;
		break;
	default:
		break;
	}

	Texture.Modify();
	Texture.PostEditChange();
	Texture.MarkPackageDirty();
}

void AddIssue(TArray<FImportIssue>& Issues, EImportIssueSeverity Severity, const FString& Code, const FString& Message)
{
	FImportIssue Issue;
	Issue.Severity = Severity;
	Issue.Code = Code;
	Issue.Message = Message;
	Issues.Add(MoveTemp(Issue));
}

bool HasBlockingIssues(const TArray<FImportIssue>& Issues)
{
	return Issues.ContainsByPredicate([](const FImportIssue& Issue)
	{
		return Issue.Severity == EImportIssueSeverity::Error;
	});
}

void ApplyFatalFailureCleanupStatus(FScanVaultImportReport& Report, bool bCleanupComplete)
{
	Report.Status = bCleanupComplete ? EImportStatus::Failed : EImportStatus::PartialImport;
}

bool ShouldWarnTextureParameterAssignmentFailed(const UTexture* ExpectedTexture, const UTexture* AssignedTexture)
{
	return AssignedTexture != ExpectedTexture;
}

FString ImportStatusToString(EImportStatus Status)
{
	switch (Status)
	{
	case EImportStatus::Success:
		return TEXT("Success");
	case EImportStatus::SuccessWithWarnings:
		return TEXT("SuccessWithWarnings");
	case EImportStatus::Failed:
		return TEXT("Failed");
	case EImportStatus::Cancelled:
		return TEXT("Cancelled");
	case EImportStatus::PartialImport:
		return TEXT("PartialImport");
	default:
		return TEXT("Failed");
	}
}

FString BuildReportText(const FScanVaultImportReport& Report)
{
	TStringBuilder<2048> Builder;
	Builder.Appendf(TEXT("Status: %s\n"), *ImportStatusToString(Report.Status));
	Builder.Appendf(TEXT("Package ID: %s\n"), *Report.PackageId);
	Builder.Appendf(TEXT("Destination: %s\n"), *Report.DestinationPath);
	Builder.Appendf(TEXT("Textures imported: %d\n"), Report.TexturesImported);
	Builder.Appendf(TEXT("Mesh imported: %s\n"), Report.bMeshImported ? TEXT("yes") : TEXT("no"));
	Builder.Appendf(TEXT("LODs imported: %d\n"), Report.LodsImported);
	Builder.Appendf(TEXT("Material Instance created: %s\n"), Report.bMaterialInstanceCreated ? TEXT("yes") : TEXT("no"));
	Builder.Appendf(TEXT("Nanite result: %s\n"), Report.bNaniteEnabled ? TEXT("enabled") : TEXT("not enabled"));
	Builder.Appendf(TEXT("Assets saved: %s\n"), Report.bAssetsSaved ? TEXT("yes") : TEXT("no"));
	Builder.Appendf(TEXT("Duration: %.2f s\n"), Report.DurationSeconds);

	if (Report.ImportedObjectPaths.Num() > 0)
	{
		Builder.Append(TEXT("\nImported assets:\n"));
		for (const FString& ObjectPath : Report.ImportedObjectPaths)
		{
			Builder.Appendf(TEXT("- %s\n"), *ObjectPath);
		}
	}

	if (Report.Issues.Num() > 0)
	{
		Builder.Append(TEXT("\nIssues:\n"));
		for (const FImportIssue& Issue : Report.Issues)
		{
			const TCHAR* SeverityText = Issue.Severity == EImportIssueSeverity::Error
				? TEXT("Error")
				: (Issue.Severity == EImportIssueSeverity::Warning ? TEXT("Warning") : TEXT("Info"));
			Builder.Appendf(TEXT("- [%s] %s: %s\n"), SeverityText, *Issue.Code, *Issue.Message);
		}
	}

	if (Report.LeftoverObjectPaths.Num() > 0)
	{
		Builder.Append(TEXT("\nCleanup leftovers:\n"));
		for (const FString& ObjectPath : Report.LeftoverObjectPaths)
		{
			Builder.Appendf(TEXT("- %s\n"), *ObjectPath);
		}
	}

	return Builder.ToString();
}

void ValidateManifestForImport(const FScanVaultManifest& Manifest, TArray<FImportIssue>& Issues)
{
	if (Manifest.SchemaVersion == 0)
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("schema.missing"), TEXT("Invalid ScanVault manifest schema."));
	}
	else if (Manifest.SchemaVersion > 1)
	{
		AddIssue(
			Issues,
			EImportIssueSeverity::Error,
			TEXT("schema.newer"),
			TEXT("This ScanVault package was created by a newer manifest schema version. UE57Editor currently supports schemaVersion 1."));
	}
	else if (Manifest.SchemaVersion < 1)
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("schema.invalid"), TEXT("Invalid ScanVault manifest schema."));
	}

	if (Manifest.PackageId.IsEmpty())
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("packageId.missing"), TEXT("Missing required packageId."));
	}
	if (Manifest.Destination.ContentPath.IsEmpty() || !IsValidContentPath(Manifest.Destination.ContentPath))
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("destination.invalid"), FString::Printf(TEXT("Invalid Unreal destination path '%s'."), *Manifest.Destination.ContentPath));
	}
	if (Manifest.Destination.AssetBaseName.IsEmpty())
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("destination.assetBaseName.missing"), TEXT("Missing destination.assetBaseName."));
	}

	if (IsScanVaultBlockingValidation(Manifest))
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("scanvault.validation.blocking"), TEXT("ScanVault manifest contains blocking validation errors."));
	}
	if (HasReadinessOverride(Manifest))
	{
		AddIssue(Issues, EImportIssueSeverity::Warning, TEXT("readiness.override"), TEXT("ScanVault readiness override is present. Confirm import explicitly."));
	}

	for (const FString& Error : Manifest.Validation.Errors)
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("scanvault.error"), Error);
	}
	for (const FString& Warning : Manifest.Validation.Warnings)
	{
		AddIssue(Issues, EImportIssueSeverity::Warning, TEXT("scanvault.warning"), Warning);
	}

	for (const FScanVaultTexture& Texture : Manifest.Textures)
	{
		if (Texture.Role == ETextureRole::Invalid)
		{
			AddIssue(Issues, EImportIssueSeverity::Error, TEXT("texture.role.invalid"), FString::Printf(TEXT("Invalid texture role '%s'."), *Texture.RoleText));
		}
		if (!FileExistsAndReadable(Texture.SourcePath))
		{
			AddIssue(Issues, EImportIssueSeverity::Error, TEXT("texture.source.missing"), FString::Printf(TEXT("Texture source is missing or unreadable: %s"), *Texture.SourcePath));
		}
		else if (!IsSupportedTextureExtension(Texture.SourcePath))
		{
			AddIssue(Issues, EImportIssueSeverity::Error, TEXT("texture.source.unsupported"), FString::Printf(TEXT("Unsupported texture extension: %s"), *Texture.SourcePath));
		}
		if (Texture.Role == ETextureRole::Roughness && NormalizeToken(Texture.MapType) == TEXT("gloss"))
		{
			AddIssue(Issues, EImportIssueSeverity::Warning, TEXT("texture.gloss"), TEXT("Original map type is Gloss. No pixel inversion was performed."));
		}
	}

	if (Manifest.Mesh.bHasMesh)
	{
		TSet<int32> SeenLods;
		for (const FScanVaultMeshLod& Lod : Manifest.Mesh.Lods)
		{
			if (Lod.Lod < 0)
			{
				AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.lod.invalid"), FString::Printf(TEXT("Invalid LOD index for source '%s'."), *Lod.SourcePath));
			}
			if (SeenLods.Contains(Lod.Lod))
			{
				AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.lod.duplicate"), FString::Printf(TEXT("Duplicate LOD index %d."), Lod.Lod));
			}
			SeenLods.Add(Lod.Lod);
			if (!FileExistsAndReadable(Lod.SourcePath))
			{
				AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.source.missing"), FString::Printf(TEXT("Mesh source is missing or unreadable: %s"), *Lod.SourcePath));
			}
			else if (!IsSupportedMeshExtension(Lod.SourcePath))
			{
				AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.source.unsupported"), FString::Printf(TEXT("Unsupported mesh extension: %s"), *Lod.SourcePath));
			}
		}

		if (Manifest.Mesh.Lods.Num() > 0 && !SeenLods.Contains(0))
		{
			AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.primary.missing"), TEXT("Mesh LOD list must contain LOD 0 for the primary mesh."));
		}

		for (int32 ExpectedLod = 0; ExpectedLod < SeenLods.Num(); ++ExpectedLod)
		{
			if (!SeenLods.Contains(ExpectedLod))
			{
				AddIssue(Issues, EImportIssueSeverity::Error, TEXT("mesh.lod.gap"), FString::Printf(TEXT("LOD list has a gap at LOD %d."), ExpectedLod));
			}
		}
	}

	if (Manifest.Options.bCreateMaterialInstance && Manifest.Material.MasterMaterialPath.IsEmpty())
	{
		AddIssue(Issues, EImportIssueSeverity::Error, TEXT("material.master.missing"), TEXT("Missing material.masterMaterialPath."));
	}
}
}
