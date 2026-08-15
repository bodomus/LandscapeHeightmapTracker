#if WITH_DEV_AUTOMATION_TESTS

#include "ContentRefreshPathUtils.h"
#include "LandscapeHeightmapTrackerCommands.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FContentRefreshPathPolicyTest,
	"LandscapeHeightmapTracker.RefreshContent.PathPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FContentRefreshPathPolicyTest::RunTest(const FString& Parameters)
{
	using LandscapeHeightmapTracker::ContentRefresh::IsProjectContentPath;

	TestTrue(TEXT("/Game is accepted"), IsProjectContentPath(TEXT("/Game")));
	TestTrue(TEXT("A direct /Game child is accepted"), IsProjectContentPath(TEXT("/Game/Environment")));
	TestTrue(TEXT("A nested /Game child is accepted"), IsProjectContentPath(TEXT("/Game/Marketplace/MyPack")));

	TestFalse(TEXT("An empty path is rejected"), IsProjectContentPath(FString()));
	TestFalse(TEXT("A relative path is rejected"), IsProjectContentPath(TEXT("Game/Environment")));
	TestFalse(TEXT("A lookalike root is rejected"), IsProjectContentPath(TEXT("/Gameplay")));
	TestFalse(TEXT("/Engine is rejected"), IsProjectContentPath(TEXT("/Engine")));
	TestFalse(TEXT("Plugin content is rejected"), IsProjectContentPath(TEXT("/SomePlugin/Content")));
	TestFalse(TEXT("Path matching is case-sensitive"), IsProjectContentPath(TEXT("/game/Environment")));

	TestTrue(
		TEXT("Refresh Content command is registered"),
		FLandscapeHeightmapTrackerCommands::Get().RefreshContent.IsValid());
	TestTrue(
		TEXT("Refresh Current Folder command is registered"),
		FLandscapeHeightmapTrackerCommands::Get().RefreshCurrentFolder.IsValid());

	return true;
}

#endif
