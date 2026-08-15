#include "ContentRefreshPathUtils.h"

namespace LandscapeHeightmapTracker
{
namespace ContentRefresh
{
bool IsProjectContentPath(const FString& Path)
{
	return Path == TEXT("/Game") || Path.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive);
}
}
}
