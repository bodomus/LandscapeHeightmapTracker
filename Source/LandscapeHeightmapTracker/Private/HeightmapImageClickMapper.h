#pragma once

#include "CoreMinimal.h"

struct FHeightmapFittedImageRect
{
	bool bIsValid = false;
	FVector2D DrawSize = FVector2D::ZeroVector;
	FVector2D DrawOffset = FVector2D::ZeroVector;
};

struct FHeightmapClickMappingResult
{
	bool bIsValid = false;
	FVector2D DisplayUV = FVector2D::ZeroVector;
	FString FailureReason;
};

class FHeightmapImageClickMapper
{
public:
	static FHeightmapFittedImageRect CalculateFittedImageRect(
		const FVector2D& WidgetSize,
		const FVector2D& ImageSize);

	static FHeightmapClickMappingResult MapLocalPositionToDisplayUV(
		const FVector2D& LocalPosition,
		const FHeightmapFittedImageRect& ImageRect);
};
