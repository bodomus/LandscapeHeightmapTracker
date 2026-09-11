#pragma once

#include "CoreMinimal.h"
#include "ScanVault/ScanVaultImportTypes.h"

namespace LandscapeHeightmapTracker::ScanVault
{
class FScanVaultManifestReader
{
public:
	static FScanVaultImportPlan ReadFromFile(const FString& ManifestPath);
	static FScanVaultImportPlan ReadFromString(const FString& ManifestPath, const FString& JsonText);
};
}
