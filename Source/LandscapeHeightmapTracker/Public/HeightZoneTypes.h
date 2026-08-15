#pragma once

#include "CoreMinimal.h"

enum class EHeightZoneMode : uint8
{
	Above,
	Below,
	ContourOnly,
	Range
};

struct FHeightZoneSettings
{
	double HeightAMeters = 800.0;
	double HeightBMeters = 1000.0;
	EHeightZoneMode Mode = EHeightZoneMode::Above;
	float FillOpacity = 0.35f;
	float ContourThickness = 2.0f;
	bool bEnabled = false;
};

struct FHeightContour
{
	TArray<FVector2D> Points;
	double BoundaryHeightMeters = 0.0;
	FLinearColor Color = FLinearColor(1.0f, 0.45f, 0.05f, 1.0f);
	bool bClosed = false;
};

struct FHeightRangeDefinition
{
	double MinHeightMeters = 0.0;
	double MaxHeightMeters = 0.0;
	FLinearColor Color = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
	bool bEnabled = true;
};

enum class EHeightRangeConflict : uint8
{
	None,
	Duplicate,
	Overlap
};

struct FHeightRangeValidationResult
{
	EHeightRangeConflict Conflict = EHeightRangeConflict::None;
	int32 ExistingRangeIndex = INDEX_NONE;
};

struct FMultiHeightRangeResult
{
	int32 Width = 0;
	int32 Height = 0;
	TArray<FHeightRangeDefinition> Ranges;
	TArray<int32> RangeIndexByPixel;
	TArray<int32> PixelCounts;
	TArray<FHeightContour> Contours;
};

struct FHeightZoneResult
{
	double MinHeightMeters = 0.0;
	double MaxHeightMeters = 0.0;
	int32 Width = 0;
	int32 Height = 0;
	TArray<uint8> Mask;
	TArray<FHeightContour> Contours;
};

