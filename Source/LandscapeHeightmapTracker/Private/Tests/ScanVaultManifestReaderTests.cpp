#if WITH_DEV_AUTOMATION_TESTS

#include "LandscapeHeightmapTrackerCommands.h"
#include "Engine/Texture2D.h"
#include "Misc/AutomationTest.h"
#include "ScanVault/ScanVaultImportTypes.h"
#include "ScanVault/ScanVaultManifestReader.h"

using namespace LandscapeHeightmapTracker::ScanVault;

namespace
{
FString MinimalManifestJson(const FString& SchemaVersionText = TEXT("1"))
{
	return FString::Printf(TEXT(R"json(
{
  "schemaVersion": %s,
  "packageId": "7bb1f6c0f4a2d91c0ef94b2f7b0cc6ae",
  "source": {
    "assetId": "asset-id",
    "name": "Forest Rock",
    "assetType": "3D Asset",
    "jsonPath": "J:/Megascans/ForestRock/asset.json",
    "assetFolderPath": "J:/Megascans/ForestRock"
  },
  "destination": {
    "baseContentPath": "/Game/Megascans",
    "contentPath": "/Game/Megascans/3D_Assets/Forest_Rock",
    "assetBaseName": "Forest_Rock",
    "originalAssetName": "Forest Rock"
  },
  "textures": [],
  "material": {
    "profileId": "surface",
    "profileName": "Surface",
    "masterMaterialPath": "/Game/Materials/M_Master.M_Master",
    "materialInstancePrefix": "MI_",
    "materialInstanceName": "MI_Forest_Rock",
    "textureParameterMappings": {
      "baseColor": "BaseColor"
    }
  },
  "options": {
    "importLods": true,
    "enableNanite": true,
    "createMaterialInstance": true
  },
  "validation": {
    "warnings": []
  }
}
)json"), *SchemaVersionText);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScanVaultCommandRegistrationTest,
	"LandscapeHeightmapTracker.ScanVault.CommandRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScanVaultCommandRegistrationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("ScanVault import command is registered"), FLandscapeHeightmapTrackerCommands::Get().ScanVaultImport.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScanVaultManifestSchemaTest,
	"LandscapeHeightmapTracker.ScanVault.Manifest.Schema",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScanVaultManifestSchemaTest::RunTest(const FString& Parameters)
{
	const FScanVaultImportPlan ValidPlan = FScanVaultManifestReader::ReadFromString(TEXT("memory.scanvault-ue.json"), MinimalManifestJson());
	TestEqual(TEXT("schemaVersion v1 parsed"), ValidPlan.Manifest.SchemaVersion, 1);
	TestEqual(TEXT("packageId preserved"), ValidPlan.Manifest.PackageId, TEXT("7bb1f6c0f4a2d91c0ef94b2f7b0cc6ae"));
	TestEqual(TEXT("mapping parsed"), ValidPlan.Manifest.Material.TextureParameterMappings.Num(), 1);
	TestEqual(TEXT("mapping role parsed"), StaticCast<int32>(ValidPlan.Manifest.Material.TextureParameterMappings[0].Role), StaticCast<int32>(ETextureRole::BaseColor));

	const FScanVaultImportPlan NewerPlan = FScanVaultManifestReader::ReadFromString(TEXT("memory.scanvault-ue.json"), MinimalManifestJson(TEXT("2")));
	TestTrue(TEXT("newer schema blocks import"), HasBlockingIssues(NewerPlan.Issues));

	const FScanVaultImportPlan MalformedPlan = FScanVaultManifestReader::ReadFromString(TEXT("memory.scanvault-ue.json"), TEXT("{"));
	TestTrue(TEXT("malformed JSON blocks import"), HasBlockingIssues(MalformedPlan.Issues));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScanVaultPolicyTest,
	"LandscapeHeightmapTracker.ScanVault.Policy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScanVaultPolicyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("/Game child path is valid"), IsValidContentPath(TEXT("/Game/Megascans/Rock")));
	TestFalse(TEXT("/Engine path is invalid"), IsValidContentPath(TEXT("/Engine/Materials")));
	TestFalse(TEXT("relative path is invalid"), IsValidContentPath(TEXT("Game/Megascans")));
	TestEqual(TEXT("base color texture name"), MakeTextureAssetName(TEXT("Forest_Rock"), ETextureRole::BaseColor), TEXT("T_Forest_Rock_BC"));
	TestEqual(TEXT("normal texture name"), MakeTextureAssetName(TEXT("Forest_Rock"), ETextureRole::Normal), TEXT("T_Forest_Rock_N"));
	TestEqual(TEXT("static mesh name"), MakeStaticMeshAssetName(TEXT("Forest_Rock")), TEXT("SM_Forest_Rock"));
	TestEqual(TEXT("role is explicit, not filename based"), StaticCast<int32>(TextureRoleFromString(TEXT("normal"))), StaticCast<int32>(ETextureRole::Normal));
	TestEqual(TEXT("unknown role rejected"), StaticCast<int32>(TextureRoleFromString(TEXT("wood_albedo_from_filename"))), StaticCast<int32>(ETextureRole::Invalid));

	FScanVaultImportReport CleanupCompleteReport;
	ApplyFatalFailureCleanupStatus(CleanupCompleteReport, true);
	TestEqual(TEXT("fatal failure with complete cleanup is Failed"), StaticCast<int32>(CleanupCompleteReport.Status), StaticCast<int32>(EImportStatus::Failed));
	TestEqual(TEXT("complete cleanup leaves no leftovers"), CleanupCompleteReport.LeftoverObjectPaths.Num(), 0);

	FScanVaultImportReport CleanupIncompleteReport;
	CleanupIncompleteReport.LeftoverObjectPaths.Add(TEXT("/Game/Megascans/Rock/T_Rock_BC.T_Rock_BC"));
	ApplyFatalFailureCleanupStatus(CleanupIncompleteReport, false);
	TestEqual(TEXT("fatal failure with incomplete cleanup is PartialImport"), StaticCast<int32>(CleanupIncompleteReport.Status), StaticCast<int32>(EImportStatus::PartialImport));
	TestEqual(TEXT("incomplete cleanup preserves leftovers"), CleanupIncompleteReport.LeftoverObjectPaths.Num(), 1);

	UTexture2D* ExpectedTexture = NewObject<UTexture2D>();
	UTexture2D* DifferentTexture = NewObject<UTexture2D>();
	TestFalse(TEXT("setter failure with matching read-back does not warn"), ShouldWarnTextureParameterAssignmentFailed(ExpectedTexture, ExpectedTexture));
	TestTrue(TEXT("read-back mismatch preserves assign_failed"), ShouldWarnTextureParameterAssignmentFailed(ExpectedTexture, DifferentTexture));
	TestTrue(TEXT("null read-back preserves assign_failed"), ShouldWarnTextureParameterAssignmentFailed(ExpectedTexture, nullptr));

	FScanVaultManifest Manifest;
	Manifest.SchemaVersion = 1;
	Manifest.PackageId = TEXT("package");
	Manifest.Destination.ContentPath = TEXT("/Game/Megascans/Rock");
	Manifest.Destination.AssetBaseName = TEXT("Rock");
	Manifest.Options.bCreateMaterialInstance = false;
	Manifest.Mesh.bHasMesh = true;

	FScanVaultMeshLod Lod0;
	Lod0.Lod = 0;
	Lod0.SourcePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("missing0.fbx"));
	Manifest.Mesh.Lods.Add(Lod0);
	FScanVaultMeshLod LodDuplicate;
	LodDuplicate.Lod = 0;
	LodDuplicate.SourcePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("missing1.fbx"));
	Manifest.Mesh.Lods.Add(LodDuplicate);

	TArray<FImportIssue> Issues;
	ValidateManifestForImport(Manifest, Issues);
	TestTrue(TEXT("duplicate LOD is blocking"), Issues.ContainsByPredicate([](const FImportIssue& Issue)
	{
		return Issue.Code == TEXT("mesh.lod.duplicate") && Issue.Severity == EImportIssueSeverity::Error;
	}));

	return true;
}

#endif
