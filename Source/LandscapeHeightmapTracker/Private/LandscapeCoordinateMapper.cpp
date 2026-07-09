#include "LandscapeCoordinateMapper.h"

bool FLandscapeTrackerBounds::IsValid() const
{
	return Max.X > Min.X && Max.Y > Min.Y;
}

FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapWorldPosition(
	const FTransform& LandscapeTransform,
	const FLandscapeTrackerBounds& LocalBounds,
	const FVector& WorldPosition,
	const FIntPoint& ImageSize,
	const FLandscapeTrackerMappingOptions& Options)
{
	FLandscapeTrackerMappingResult Result = MapLocalPosition(
		LocalBounds,
		LandscapeTransform.InverseTransformPosition(WorldPosition),
		ImageSize,
		Options);

	Result.WorldPosition = WorldPosition;
	return Result;
}

FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapLocalPosition(
	const FLandscapeTrackerBounds& LocalBounds,
	const FVector& LocalPosition,
	const FIntPoint& ImageSize,
	const FLandscapeTrackerMappingOptions& Options)
{
	FLandscapeTrackerMappingResult Result;
	Result.LocalPosition = LocalPosition;

	if (!LocalBounds.IsValid())
	{
		Result.FailureReason = TEXT("Invalid Landscape bounds.");
		return Result;
	}

	if (ImageSize.X <= 0 || ImageSize.Y <= 0)
	{
		Result.FailureReason = TEXT("No valid heightmap image size.");
		return Result;
	}

	const double Width = LocalBounds.Max.X - LocalBounds.Min.X;
	const double Height = LocalBounds.Max.Y - LocalBounds.Min.Y;
	double U = (LocalPosition.X - LocalBounds.Min.X) / Width;
	double V = (LocalPosition.Y - LocalBounds.Min.Y) / Height;

	const bool bInside =
		U >= -Options.BoundsTolerance &&
		U <= 1.0 + Options.BoundsTolerance &&
		V >= -Options.BoundsTolerance &&
		V <= 1.0 + Options.BoundsTolerance;

	Result.bIsInsideBounds = bInside;

	if (!bInside && !Options.bClampToBounds)
	{
		Result.FailureReason = TEXT("Point is outside Landscape bounds.");
		return Result;
	}

	U = FMath::Clamp(U, 0.0, 1.0);
	V = FMath::Clamp(V, 0.0, 1.0);

	if (Options.bFlipX)
	{
		U = 1.0 - U;
	}

	if (Options.bFlipY)
	{
		V = 1.0 - V;
	}

	Result.NormalizedUV = FVector2D(U, V);
	Result.Pixel = UVToPixel(Result.NormalizedUV, ImageSize);
	Result.bIsValid = true;
	return Result;
}

FLandscapeTrackerReverseMappingResult FLandscapeCoordinateMapper::MapUVToLocalPosition(
	const FLandscapeTrackerBounds& LocalBounds,
	const FVector2D& DisplayUV,
	const FLandscapeTrackerMappingOptions& Options)
{
	FLandscapeTrackerReverseMappingResult Result;
	Result.DisplayUV = DisplayUV;

	if (!LocalBounds.IsValid())
	{
		Result.FailureReason = TEXT("Invalid Landscape bounds.");
		return Result;
	}

	const bool bInside =
		DisplayUV.X >= -Options.BoundsTolerance &&
		DisplayUV.X <= 1.0 + Options.BoundsTolerance &&
		DisplayUV.Y >= -Options.BoundsTolerance &&
		DisplayUV.Y <= 1.0 + Options.BoundsTolerance;

	if (!bInside && !Options.bClampToBounds)
	{
		Result.FailureReason = TEXT("Point is outside heightmap UV bounds.");
		return Result;
	}

	double U = FMath::Clamp(static_cast<double>(DisplayUV.X), 0.0, 1.0);
	double V = FMath::Clamp(static_cast<double>(DisplayUV.Y), 0.0, 1.0);

	if (Options.bFlipX)
	{
		U = 1.0 - U;
	}

	if (Options.bFlipY)
	{
		V = 1.0 - V;
	}

	const double Width = LocalBounds.Max.X - LocalBounds.Min.X;
	const double Height = LocalBounds.Max.Y - LocalBounds.Min.Y;
	Result.LandscapeUV = FVector2D(U, V);
	Result.LocalPosition = FVector(
		LocalBounds.Min.X + U * Width,
		LocalBounds.Min.Y + V * Height,
		0.0);
	Result.bIsValid = true;
	return Result;
}

FIntPoint FLandscapeCoordinateMapper::UVToPixel(const FVector2D& UV, const FIntPoint& ImageSize)
{
	return FIntPoint(
		FMath::Clamp(FMath::RoundToInt(UV.X * static_cast<double>(ImageSize.X - 1)), 0, ImageSize.X - 1),
		FMath::Clamp(FMath::RoundToInt(UV.Y * static_cast<double>(ImageSize.Y - 1)), 0, ImageSize.Y - 1));
}
