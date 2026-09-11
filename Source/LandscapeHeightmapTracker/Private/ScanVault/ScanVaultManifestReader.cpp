#include "ScanVault/ScanVaultManifestReader.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace LandscapeHeightmapTracker::ScanVault
{
namespace
{
bool GetObject(const TSharedPtr<FJsonObject>& Object, const FString& Field, TSharedPtr<FJsonObject>& OutObject)
{
	if (!Object.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* FoundObject = nullptr;
	if (Object->TryGetObjectField(FStringView(*Field, Field.Len()), FoundObject) && FoundObject)
	{
		OutObject = *FoundObject;
		return OutObject.IsValid();
	}
	return false;
}

void GetString(const TSharedPtr<FJsonObject>& Object, const FString& Field, FString& OutValue)
{
	if (Object.IsValid())
	{
		Object->TryGetStringField(Field, OutValue);
	}
}

bool GetBoolOrDefault(const TSharedPtr<FJsonObject>& Object, const FString& Field, bool DefaultValue)
{
	bool Value = DefaultValue;
	if (Object.IsValid())
	{
		Object->TryGetBoolField(Field, Value);
	}
	return Value;
}

void ParseStringArray(const TSharedPtr<FJsonObject>& Object, const FString& Field, TArray<FString>& OutValues)
{
	const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
	if (!Object.IsValid() || !Object->TryGetArrayField(Field, Values))
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& Value : *Values)
	{
		FString Text;
		if (Value.IsValid() && Value->TryGetString(Text))
		{
			OutValues.Add(Text);
		}
		else if (Value.IsValid() && Value->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> ValueObject = Value->AsObject();
			if (ValueObject.IsValid())
			{
				GetString(ValueObject, TEXT("message"), Text);
				if (Text.IsEmpty())
				{
					GetString(ValueObject, TEXT("text"), Text);
				}
				if (Text.IsEmpty())
				{
					GetString(ValueObject, TEXT("reason"), Text);
				}
				if (!Text.IsEmpty())
				{
					OutValues.Add(Text);
				}
			}
		}
	}
}

void ParseValidation(const TSharedPtr<FJsonObject>& Root, FScanVaultValidationState& OutValidation)
{
	TSharedPtr<FJsonObject> ValidationObject;
	if (!GetObject(Root, TEXT("validation"), ValidationObject))
	{
		return;
	}

	ParseStringArray(ValidationObject, TEXT("errors"), OutValidation.Errors);
	ParseStringArray(ValidationObject, TEXT("warnings"), OutValidation.Warnings);
	ParseStringArray(ValidationObject, TEXT("infos"), OutValidation.Infos);
	ParseStringArray(ValidationObject, TEXT("information"), OutValidation.Infos);
}

void ParseSource(const TSharedPtr<FJsonObject>& Root, FScanVaultSource& OutSource)
{
	TSharedPtr<FJsonObject> SourceObject;
	if (!GetObject(Root, TEXT("source"), SourceObject))
	{
		return;
	}

	GetString(SourceObject, TEXT("assetId"), OutSource.AssetId);
	GetString(SourceObject, TEXT("name"), OutSource.Name);
	GetString(SourceObject, TEXT("assetType"), OutSource.AssetType);
	GetString(SourceObject, TEXT("jsonPath"), OutSource.JsonPath);
	GetString(SourceObject, TEXT("assetFolderPath"), OutSource.AssetFolderPath);
}

void ParseDestination(const TSharedPtr<FJsonObject>& Root, FScanVaultDestination& OutDestination)
{
	TSharedPtr<FJsonObject> DestinationObject;
	if (!GetObject(Root, TEXT("destination"), DestinationObject))
	{
		return;
	}

	GetString(DestinationObject, TEXT("baseContentPath"), OutDestination.BaseContentPath);
	GetString(DestinationObject, TEXT("contentPath"), OutDestination.ContentPath);
	GetString(DestinationObject, TEXT("assetBaseName"), OutDestination.AssetBaseName);
	GetString(DestinationObject, TEXT("originalAssetName"), OutDestination.OriginalAssetName);
}

void ParseMesh(const TSharedPtr<FJsonObject>& Root, FScanVaultMesh& OutMesh)
{
	TSharedPtr<FJsonObject> MeshObject;
	if (!GetObject(Root, TEXT("mesh"), MeshObject))
	{
		return;
	}

	OutMesh.bHasMesh = true;
	GetString(MeshObject, TEXT("primaryVariant"), OutMesh.PrimaryVariant);

	const TArray<TSharedPtr<FJsonValue>>* LodValues = nullptr;
	if (!MeshObject->TryGetArrayField(TEXT("lods"), LodValues))
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& LodValue : *LodValues)
	{
		if (!LodValue.IsValid() || LodValue->Type != EJson::Object)
		{
			continue;
		}

		const TSharedPtr<FJsonObject> LodObject = LodValue->AsObject();
		FScanVaultMeshLod Lod;
		GetString(LodObject, TEXT("variant"), Lod.Variant);
		LodObject->TryGetNumberField(TEXT("lod"), Lod.Lod);
		GetString(LodObject, TEXT("sourcePath"), Lod.SourcePath);
		GetString(LodObject, TEXT("format"), Lod.Format);
		OutMesh.Lods.Add(MoveTemp(Lod));
	}

	OutMesh.Lods.Sort([](const FScanVaultMeshLod& A, const FScanVaultMeshLod& B)
	{
		return A.Lod < B.Lod;
	});
}

void ParseTextures(const TSharedPtr<FJsonObject>& Root, TArray<FScanVaultTexture>& OutTextures)
{
	const TArray<TSharedPtr<FJsonValue>>* TextureValues = nullptr;
	if (!Root.IsValid() || !Root->TryGetArrayField(TEXT("textures"), TextureValues))
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& TextureValue : *TextureValues)
	{
		if (!TextureValue.IsValid() || TextureValue->Type != EJson::Object)
		{
			continue;
		}

		const TSharedPtr<FJsonObject> TextureObject = TextureValue->AsObject();
		FScanVaultTexture Texture;
		GetString(TextureObject, TEXT("role"), Texture.RoleText);
		Texture.Role = TextureRoleFromString(Texture.RoleText);
		GetString(TextureObject, TEXT("sourcePath"), Texture.SourcePath);
		GetString(TextureObject, TEXT("mapType"), Texture.MapType);
		GetString(TextureObject, TEXT("setKind"), Texture.SetKind);
		TextureObject->TryGetNumberField(TEXT("resolution"), Texture.Resolution);
		GetString(TextureObject, TEXT("format"), Texture.Format);
		OutTextures.Add(MoveTemp(Texture));
	}
}

void ParseMappingObject(const TSharedPtr<FJsonObject>& MappingObject, TArray<FScanVaultTextureParameterMapping>& OutMappings)
{
	if (!MappingObject.IsValid())
	{
		return;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : MappingObject->Values)
	{
		FString ParameterName;
		if (Pair.Value.IsValid() && Pair.Value->TryGetString(ParameterName) && !ParameterName.IsEmpty())
		{
			FScanVaultTextureParameterMapping Mapping;
			Mapping.RoleText = Pair.Key;
			Mapping.Role = TextureRoleFromString(Pair.Key);
			Mapping.ParameterName = FName(*ParameterName);
			OutMappings.Add(MoveTemp(Mapping));
		}
	}
}

void ParseMappingArray(const TArray<TSharedPtr<FJsonValue>>& MappingValues, TArray<FScanVaultTextureParameterMapping>& OutMappings)
{
	for (const TSharedPtr<FJsonValue>& MappingValue : MappingValues)
	{
		if (!MappingValue.IsValid() || MappingValue->Type != EJson::Object)
		{
			continue;
		}

		const TSharedPtr<FJsonObject> MappingObject = MappingValue->AsObject();
		FString RoleText;
		FString ParameterName;
		GetString(MappingObject, TEXT("role"), RoleText);
		GetString(MappingObject, TEXT("parameterName"), ParameterName);
		if (ParameterName.IsEmpty())
		{
			GetString(MappingObject, TEXT("parameter"), ParameterName);
		}

		if (!RoleText.IsEmpty() && !ParameterName.IsEmpty())
		{
			FScanVaultTextureParameterMapping Mapping;
			Mapping.RoleText = RoleText;
			Mapping.Role = TextureRoleFromString(RoleText);
			Mapping.ParameterName = FName(*ParameterName);
			OutMappings.Add(MoveTemp(Mapping));
		}
	}
}

void ParseMappingsFromField(const TSharedPtr<FJsonObject>& MaterialObject, const FString& Field, TArray<FScanVaultTextureParameterMapping>& OutMappings)
{
	if (!MaterialObject.IsValid())
	{
		return;
	}

	TSharedPtr<FJsonObject> MappingObject;
	const TSharedPtr<FJsonObject>* FoundObject = nullptr;
	if (MaterialObject->TryGetObjectField(FStringView(*Field, Field.Len()), FoundObject) && FoundObject)
	{
		MappingObject = *FoundObject;
		ParseMappingObject(MappingObject, OutMappings);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* MappingValues = nullptr;
	if (MaterialObject->TryGetArrayField(Field, MappingValues))
	{
		ParseMappingArray(*MappingValues, OutMappings);
	}
}

void ParseMaterial(const TSharedPtr<FJsonObject>& Root, FScanVaultMaterial& OutMaterial)
{
	TSharedPtr<FJsonObject> MaterialObject;
	if (!GetObject(Root, TEXT("material"), MaterialObject))
	{
		return;
	}

	GetString(MaterialObject, TEXT("profileId"), OutMaterial.ProfileId);
	GetString(MaterialObject, TEXT("profileID"), OutMaterial.ProfileId);
	GetString(MaterialObject, TEXT("profileName"), OutMaterial.ProfileName);
	GetString(MaterialObject, TEXT("masterMaterialPath"), OutMaterial.MasterMaterialPath);
	GetString(MaterialObject, TEXT("materialInstancePrefix"), OutMaterial.MaterialInstancePrefix);
	GetString(MaterialObject, TEXT("materialInstanceName"), OutMaterial.MaterialInstanceName);
	ParseStringArray(MaterialObject, TEXT("compatibleAssetTypes"), OutMaterial.CompatibleAssetTypes);

	ParseMappingsFromField(MaterialObject, TEXT("textureParameterMappings"), OutMaterial.TextureParameterMappings);
	if (OutMaterial.TextureParameterMappings.IsEmpty())
	{
		ParseMappingsFromField(MaterialObject, TEXT("parameterMappings"), OutMaterial.TextureParameterMappings);
	}
	if (OutMaterial.TextureParameterMappings.IsEmpty())
	{
		ParseMappingsFromField(MaterialObject, TEXT("mappings"), OutMaterial.TextureParameterMappings);
	}
}

void ParseOptions(const TSharedPtr<FJsonObject>& Root, FScanVaultOptions& OutOptions)
{
	TSharedPtr<FJsonObject> OptionsObject;
	if (!GetObject(Root, TEXT("options"), OptionsObject))
	{
		return;
	}

	OutOptions.bImportLods = GetBoolOrDefault(OptionsObject, TEXT("importLods"), OutOptions.bImportLods);
	OutOptions.bEnableNanite = GetBoolOrDefault(OptionsObject, TEXT("enableNanite"), OutOptions.bEnableNanite);
	OutOptions.bCreateMaterialInstance = GetBoolOrDefault(OptionsObject, TEXT("createMaterialInstance"), OutOptions.bCreateMaterialInstance);
	OutOptions.bReadinessOverride = GetBoolOrDefault(OptionsObject, TEXT("readinessOverride"), OutOptions.bReadinessOverride);
}

void ParseManifestObject(const TSharedPtr<FJsonObject>& Root, FScanVaultManifest& OutManifest)
{
	if (!Root.IsValid())
	{
		return;
	}

	Root->TryGetNumberField(TEXT("schemaVersion"), OutManifest.SchemaVersion);
	GetString(Root, TEXT("packageId"), OutManifest.PackageId);
	ParseSource(Root, OutManifest.Source);
	ParseDestination(Root, OutManifest.Destination);
	ParseMesh(Root, OutManifest.Mesh);
	ParseTextures(Root, OutManifest.Textures);
	ParseMaterial(Root, OutManifest.Material);
	ParseOptions(Root, OutManifest.Options);
	ParseValidation(Root, OutManifest.Validation);
}

void ValidateRequiredSections(const TSharedPtr<FJsonObject>& Root, TArray<FImportIssue>& Issues)
{
	static const TCHAR* RequiredFields[] =
	{
		TEXT("schemaVersion"),
		TEXT("packageId"),
		TEXT("source"),
		TEXT("destination"),
		TEXT("textures"),
		TEXT("material"),
		TEXT("options"),
		TEXT("validation")
	};

	for (const TCHAR* Field : RequiredFields)
	{
		if (!Root->HasField(Field))
		{
			AddIssue(
				Issues,
				EImportIssueSeverity::Error,
				TEXT("manifest.section.missing"),
				FString::Printf(TEXT("Missing required manifest field '%s'."), Field));
		}
	}
}
}

FScanVaultImportPlan FScanVaultManifestReader::ReadFromFile(const FString& ManifestPath)
{
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ManifestPath))
	{
		FScanVaultImportPlan Plan;
		Plan.ManifestPath = ManifestPath;
		AddIssue(Plan.Issues, EImportIssueSeverity::Error, TEXT("manifest.read"), FString::Printf(TEXT("Failed to read manifest file: %s"), *ManifestPath));
		return Plan;
	}

	return ReadFromString(ManifestPath, JsonText);
}

FScanVaultImportPlan FScanVaultManifestReader::ReadFromString(const FString& ManifestPath, const FString& JsonText)
{
	FScanVaultImportPlan Plan;
	Plan.ManifestPath = ManifestPath;

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		AddIssue(Plan.Issues, EImportIssueSeverity::Error, TEXT("manifest.json.malformed"), TEXT("Malformed ScanVault manifest JSON."));
		return Plan;
	}

	ValidateRequiredSections(Root, Plan.Issues);
	ParseManifestObject(Root, Plan.Manifest);
	ValidateManifestForImport(Plan.Manifest, Plan.Issues);
	Plan.bCanImport = !HasBlockingIssues(Plan.Issues);
	return Plan;
}
}
