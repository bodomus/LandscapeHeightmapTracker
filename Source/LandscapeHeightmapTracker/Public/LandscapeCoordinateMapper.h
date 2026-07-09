#pragma once

#include "CoreMinimal.h"

struct FLandscapeTrackerBounds
{
	FVector2D Min = FVector2D::ZeroVector;
	FVector2D Max = FVector2D::ZeroVector;

	bool IsValid() const;
};

struct FLandscapeTrackerMappingOptions
{
	bool bFlipX = false;
	bool bFlipY = true;
	bool bClampToBounds = false;
	float BoundsTolerance = KINDA_SMALL_NUMBER;
};

struct FLandscapeTrackerMappingResult
{
	bool bIsValid = false;
	bool bIsInsideBounds = false;
	FVector WorldPosition = FVector::ZeroVector;
	FVector LocalPosition = FVector::ZeroVector;
	FVector2D NormalizedUV = FVector2D::ZeroVector;
	FIntPoint Pixel = FIntPoint::ZeroValue;
	FString FailureReason;
};

struct FLandscapeTrackerReverseMappingResult
{
	bool bIsValid = false;
	FVector2D DisplayUV = FVector2D::ZeroVector;
	FVector2D LandscapeUV = FVector2D::ZeroVector;
	FVector LocalPosition = FVector::ZeroVector;
	FString FailureReason;
};

class FLandscapeCoordinateMapper
{
public:
	static FLandscapeTrackerMappingResult MapWorldPosition(
		const FTransform& LandscapeTransform,
		const FLandscapeTrackerBounds& LocalBounds,
		const FVector& WorldPosition,
		const FIntPoint& ImageSize,
		const FLandscapeTrackerMappingOptions& Options);

	static FLandscapeTrackerMappingResult MapLocalPosition(
		const FLandscapeTrackerBounds& LocalBounds,
		const FVector& LocalPosition,
		const FIntPoint& ImageSize,
		const FLandscapeTrackerMappingOptions& Options);

	static FLandscapeTrackerReverseMappingResult MapUVToLocalPosition(
		const FLandscapeTrackerBounds& LocalBounds,
		const FVector2D& DisplayUV,
		const FLandscapeTrackerMappingOptions& Options);

	static FIntPoint UVToPixel(const FVector2D& UV, const FIntPoint& ImageSize);
};
