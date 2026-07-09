#include "LandscapeSurfaceTraceHelper.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeSurfaceTraceHelper, Log, All);

FLandscapeVerticalTraceSegment FLandscapeSurfaceTraceHelper::BuildVerticalTraceSegment(
	const FVector& WorldXY,
	double TraceHalfHeight)
{
	FLandscapeVerticalTraceSegment Segment;

	if (TraceHalfHeight <= 0.0)
	{
		Segment.FailureReason = TEXT("Trace distance must be positive.");
		return Segment;
	}

	Segment.Start = FVector(WorldXY.X, WorldXY.Y, WorldXY.Z + TraceHalfHeight);
	Segment.End = FVector(WorldXY.X, WorldXY.Y, WorldXY.Z - TraceHalfHeight);
	Segment.bIsValid = Segment.Start.Z > Segment.End.Z;
	if (!Segment.bIsValid)
	{
		Segment.FailureReason = TEXT("Vertical trace segment is invalid.");
	}
	return Segment;
}

bool FLandscapeSurfaceTraceHelper::HitBelongsToAssignedLandscape(
	const ALandscapeProxy* AssignedLandscape,
	const AActor* HitActor,
	const UPrimitiveComponent* HitComponent)
{
	if (!AssignedLandscape)
	{
		return false;
	}

	if (HitActor == AssignedLandscape)
	{
		return true;
	}

	return HitComponent && HitComponent->GetOwner() == AssignedLandscape;
}

FLandscapeSurfaceTraceResult FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface(
	UWorld* World,
	const ALandscapeProxy* AssignedLandscape,
	const FVector& WorldXY,
	double TraceHalfHeight)
{
	FLandscapeSurfaceTraceResult Result;

	if (!World)
	{
		Result.FailureReason = TEXT("Could not resolve Landscape surface at selected XY.");
		return Result;
	}

	if (!AssignedLandscape)
	{
		Result.FailureReason = TEXT("No Landscape assigned.");
		return Result;
	}

	const FLandscapeVerticalTraceSegment Segment = BuildVerticalTraceSegment(WorldXY, TraceHalfHeight);
	if (!Segment.bIsValid)
	{
		Result.FailureReason = Segment.FailureReason;
		return Result;
	}

	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandscapeHeightmapTrackerSurface), true);
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.bTraceComplex = true;

	World->LineTraceMultiByChannel(Hits, Segment.Start, Segment.End, ECC_Visibility, QueryParams);
	Result.HitCount = Hits.Num();

	UE_LOG(
		LogLandscapeSurfaceTraceHelper,
		Verbose,
		TEXT("Surface trace. Start=%s End=%s HitCount=%d"),
		*Segment.Start.ToCompactString(),
		*Segment.End.ToCompactString(),
		Hits.Num());

	for (const FHitResult& Hit : Hits)
	{
		if (HitBelongsToAssignedLandscape(AssignedLandscape, Hit.GetActor(), Hit.GetComponent()))
		{
			Result.WorldPosition = Hit.ImpactPoint;
			Result.bIsValid = true;
			UE_LOG(
				LogLandscapeSurfaceTraceHelper,
				Verbose,
				TEXT("Accepted Landscape surface hit. Actor=%s Component=%s Impact=%s"),
				Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"),
				Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("None"),
				*Hit.ImpactPoint.ToCompactString());
			return Result;
		}
	}

	Result.FailureReason = TEXT("No hit on Assigned Landscape.");
	return Result;
}
