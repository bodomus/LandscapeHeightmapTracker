#pragma once

#include "CoreMinimal.h"

class ALandscapeProxy;
class AActor;
class UPrimitiveComponent;
class UWorld;

struct FLandscapeVerticalTraceSegment
{
	bool bIsValid = false;
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	FString FailureReason;
};

struct FLandscapeSurfaceTraceResult
{
	bool bIsValid = false;
	FVector WorldPosition = FVector::ZeroVector;
	int32 HitCount = 0;
	FString FailureReason;
};

class FLandscapeSurfaceTraceHelper
{
public:
	static constexpr double DefaultTraceHalfHeight = 1000000.0;

	static FLandscapeVerticalTraceSegment BuildVerticalTraceSegment(
		const FVector& WorldXY,
		double TraceHalfHeight = DefaultTraceHalfHeight);

	static bool HitBelongsToAssignedLandscape(
		const ALandscapeProxy* AssignedLandscape,
		const AActor* HitActor,
		const UPrimitiveComponent* HitComponent);

	static FLandscapeSurfaceTraceResult TraceAssignedLandscapeSurface(
		UWorld* World,
		const ALandscapeProxy* AssignedLandscape,
		const FVector& WorldXY,
		double TraceHalfHeight = DefaultTraceHalfHeight);
};
