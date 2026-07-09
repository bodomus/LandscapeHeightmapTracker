#include "HeightmapImageClickMapper.h"

FHeightmapFittedImageRect FHeightmapImageClickMapper::CalculateFittedImageRect(
	const FVector2D& WidgetSize,
	const FVector2D& ImageSize)
{
	FHeightmapFittedImageRect Result;

	if (WidgetSize.X <= 0.0 || WidgetSize.Y <= 0.0 || ImageSize.X <= 0.0 || ImageSize.Y <= 0.0)
	{
		return Result;
	}

	const double ImageAspect = ImageSize.X / ImageSize.Y;
	const double WidgetAspect = WidgetSize.X / WidgetSize.Y;

	Result.DrawSize = WidgetSize;
	if (WidgetAspect > ImageAspect)
	{
		Result.DrawSize.X = WidgetSize.Y * ImageAspect;
	}
	else
	{
		Result.DrawSize.Y = WidgetSize.X / ImageAspect;
	}

	if (Result.DrawSize.X <= 0.0 || Result.DrawSize.Y <= 0.0)
	{
		return Result;
	}

	Result.DrawOffset = (WidgetSize - Result.DrawSize) * 0.5;
	Result.bIsValid = true;
	return Result;
}

FHeightmapClickMappingResult FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(
	const FVector2D& LocalPosition,
	const FHeightmapFittedImageRect& ImageRect)
{
	FHeightmapClickMappingResult Result;

	if (!ImageRect.bIsValid)
	{
		Result.FailureReason = TEXT("No heightmap loaded.");
		return Result;
	}

	const FVector2D RelativePosition = LocalPosition - ImageRect.DrawOffset;
	if (RelativePosition.X < 0.0 || RelativePosition.Y < 0.0 ||
		RelativePosition.X > ImageRect.DrawSize.X || RelativePosition.Y > ImageRect.DrawSize.Y)
	{
		Result.FailureReason = TEXT("Click is outside the heightmap image area.");
		return Result;
	}

	Result.DisplayUV = FVector2D(
		RelativePosition.X / ImageRect.DrawSize.X,
		RelativePosition.Y / ImageRect.DrawSize.Y);
	Result.bIsValid = true;
	return Result;
}
