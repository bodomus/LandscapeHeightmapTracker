#pragma once

#include "CoreMinimal.h"
#include "ScanVault/ScanVaultImportTypes.h"

namespace LandscapeHeightmapTracker::ScanVault
{
class FScanVaultImporter
{
public:
	static FScanVaultImportReport Import(const FScanVaultImportRequest& Request);
};
}
