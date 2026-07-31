#pragma once

#include "CoreMinimal.h"

enum class EHeightZoneMode : uint8
{
	Above,
	Below,
	ContourOnly
};

struct FHeightZoneSettings
{
	double TargetHeightMeters = 800.0;
	EHeightZoneMode Mode = EHeightZoneMode::Above;
	float FillOpacity = 0.35f;
	float ContourThickness = 2.0f;
	bool bEnabled = false;
};

struct FHeightContour
{
	TArray<FVector2D> Points;
	bool bClosed = false;
};

struct FHeightZoneResult
{
	double TargetHeightMeters = 0.0;
	int32 Width = 0;
	int32 Height = 0;
	TArray<uint8> Mask;
	TArray<FHeightContour> Contours;
};

