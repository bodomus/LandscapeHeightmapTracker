#include "HeightmapImageInfoAnalyzer.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapImageInfoAnalyzer8BitTest, "LandscapeHeightmapTracker.HeightmapInfo.Grayscale8Bit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapImageInfoAnalyzer8BitTest::RunTest(const FString& Parameters)
{
	const TArray64<uint8> Samples = {0, 1, 1, 127, 255, 255};
	const FHeightmapGrayscaleInfo Info = FHeightmapImageInfoAnalyzer::Analyze(Samples, 8);

	TestTrue(TEXT("Analysis is valid"), Info.bIsValid);
	TestEqual(TEXT("Possible levels"), Info.PossibleLevelCount, 256);
	TestEqual(TEXT("Unique levels"), Info.UniqueLevelCount, 4);
	TestEqual(TEXT("Minimum"), Info.MinValue, static_cast<uint16>(0));
	TestEqual(TEXT("Maximum"), Info.MaxValue, static_cast<uint16>(255));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapImageInfoAnalyzer16BitTest, "LandscapeHeightmapTracker.HeightmapInfo.Grayscale16Bit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapImageInfoAnalyzer16BitTest::RunTest(const FString& Parameters)
{
	const TArray<uint16> Values = {0, 1, 1, 32768, 65535};
	TArray64<uint8> Samples;
	Samples.SetNumUninitialized(Values.Num() * sizeof(uint16));
	FMemory::Memcpy(Samples.GetData(), Values.GetData(), Samples.Num());

	const FHeightmapGrayscaleInfo Info = FHeightmapImageInfoAnalyzer::Analyze(Samples, 16);
	TestTrue(TEXT("Analysis is valid"), Info.bIsValid);
	TestEqual(TEXT("Possible levels"), Info.PossibleLevelCount, 65536);
	TestEqual(TEXT("Unique levels"), Info.UniqueLevelCount, 4);
	TestEqual(TEXT("Minimum"), Info.MinValue, static_cast<uint16>(0));
	TestEqual(TEXT("Maximum"), Info.MaxValue, static_cast<uint16>(65535));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapImageInfoAnalyzerInvalidInputTest, "LandscapeHeightmapTracker.HeightmapInfo.InvalidInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHeightmapImageInfoAnalyzerInvalidInputTest::RunTest(const FString& Parameters)
{
	const TArray64<uint8> EmptySamples;
	TestFalse(TEXT("Empty data is invalid"), FHeightmapImageInfoAnalyzer::Analyze(EmptySamples, 16).bIsValid);

	const TArray64<uint8> MisalignedSamples = {0};
	TestFalse(TEXT("Misaligned 16-bit data is invalid"), FHeightmapImageInfoAnalyzer::Analyze(MisalignedSamples, 16).bIsValid);
	TestFalse(TEXT("Unsupported bit depth is invalid"), FHeightmapImageInfoAnalyzer::Analyze(MisalignedSamples, 12).bIsValid);
	return true;
}
