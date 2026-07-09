#include "ViewportTraceRayBuilder.h"

FViewportTraceSegment FViewportTraceRayBuilder::BuildTraceSegment(
	bool bIsPerspective,
	const FVector& Origin,
	const FVector& Direction,
	double TraceDistance)
{
	FViewportTraceSegment Segment;
	Segment.Origin = Origin;

	if (TraceDistance <= 0.0)
	{
		Segment.FailureReason = TEXT("Trace distance must be positive.");
		return Segment;
	}

	if (Direction.IsNearlyZero())
	{
		Segment.FailureReason = TEXT("Viewport cursor direction is invalid.");
		return Segment;
	}

	Segment.Direction = Direction.GetSafeNormal();
	Segment.Start = Origin;
	Segment.End = Origin + Segment.Direction * TraceDistance;

	if (!bIsPerspective)
	{
		const double HalfTraceDistance = TraceDistance * 0.5;
		Segment.Start = Origin - Segment.Direction * HalfTraceDistance;
		Segment.End = Origin + Segment.Direction * HalfTraceDistance;
	}

	Segment.bIsValid = true;
	return Segment;
}
