#pragma once

#include "CoreMinimal.h"

struct FViewportTraceSegment
{
	bool bIsValid = false;
	FVector Origin = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	FString FailureReason;
};

class FViewportTraceRayBuilder
{
public:
	static constexpr double DefaultTraceDistance = 100000000.0;

	static FViewportTraceSegment BuildTraceSegment(
		bool bIsPerspective,
		const FVector& Origin,
		const FVector& Direction,
		double TraceDistance = DefaultTraceDistance);
};
