#include "ContourBoxPlacement.h"

namespace
{
constexpr double CentimetersPerMeter = 100.0;

struct FPolylineData
{
	TArray<FVector> Points;
	TArray<double> SegmentStarts;
	TArray<double> SegmentLengths;
	double TotalLength = 0.0;
	bool bClosed = false;
};

bool IsFiniteVector(const FVector& Point)
{
	return FMath::IsFinite(Point.X) && FMath::IsFinite(Point.Y) && FMath::IsFinite(Point.Z);
}

bool BuildPolyline(const TArray<FVector>& InputPoints, bool bClosed, FPolylineData& OutData)
{
	for (const FVector& Point : InputPoints)
	{
		if (!IsFiniteVector(Point))
		{
			return false;
		}
		const FVector Flattened(Point.X, Point.Y, 0.0);
		if (OutData.Points.IsEmpty() || !Flattened.Equals(OutData.Points.Last(), KINDA_SMALL_NUMBER))
		{
			OutData.Points.Add(Flattened);
		}
	}

	if (bClosed && OutData.Points.Num() > 1 && OutData.Points[0].Equals(OutData.Points.Last(), KINDA_SMALL_NUMBER))
	{
		OutData.Points.Pop();
	}
	OutData.bClosed = bClosed;
	if (OutData.Points.Num() < 2 || (bClosed && OutData.Points.Num() < 3))
	{
		return true;
	}

	const int32 SegmentCount = bClosed ? OutData.Points.Num() : OutData.Points.Num() - 1;
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const FVector& A = OutData.Points[SegmentIndex];
		const FVector& B = OutData.Points[(SegmentIndex + 1) % OutData.Points.Num()];
		const double Length = FVector::Dist2D(A, B);
		if (Length <= KINDA_SMALL_NUMBER)
		{
			continue;
		}
		OutData.SegmentStarts.Add(OutData.TotalLength);
		OutData.SegmentLengths.Add(Length);
		OutData.TotalLength += Length;
	}
	return true;
}

bool SamplePolyline(const FPolylineData& Polyline, double Distance, FVector& OutPoint)
{
	if (Polyline.SegmentLengths.IsEmpty() || Polyline.TotalLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}
	Distance = FMath::Clamp(Distance, 0.0, Polyline.TotalLength);
	for (int32 SegmentIndex = 0; SegmentIndex < Polyline.SegmentLengths.Num(); ++SegmentIndex)
	{
		const double SegmentStart = Polyline.SegmentStarts[SegmentIndex];
		const double SegmentLength = Polyline.SegmentLengths[SegmentIndex];
		const bool bLast = SegmentIndex == Polyline.SegmentLengths.Num() - 1;
		if (Distance <= SegmentStart + SegmentLength || bLast)
		{
			const FVector& A = Polyline.Points[SegmentIndex];
			const FVector& B = Polyline.Points[(SegmentIndex + 1) % Polyline.Points.Num()];
			const double Alpha = FMath::Clamp((Distance - SegmentStart) / SegmentLength, 0.0, 1.0);
			OutPoint = FMath::Lerp(A, B, Alpha);
			return true;
		}
	}
	return false;
}
}

FContourBoxPlacementResult FContourBoxPlacement::Generate(
	const TArray<FVector>& WorldPoints,
	bool bClosed,
	double ContourWorldZMeters,
	const FContourBoxSettings& Settings)
{
	FContourBoxPlacementResult Result;
	if (!FMath::IsFinite(Settings.BoxLengthMeters) || Settings.BoxLengthMeters <= 0.0 ||
		!FMath::IsFinite(Settings.BoxHeightMeters) || Settings.BoxHeightMeters <= 0.0 ||
		!FMath::IsFinite(Settings.BoxThicknessMeters) || Settings.BoxThicknessMeters <= 0.0 ||
		!FMath::IsFinite(Settings.GapLengthMeters) || Settings.GapLengthMeters < 0.0 ||
		!FMath::IsFinite(Settings.ZOffsetMeters) || !FMath::IsFinite(ContourWorldZMeters) ||
		Settings.MaxInstances <= 0)
	{
		Result.Error = TEXT("Box dimensions must be finite and positive; gap must be finite and non-negative.");
		return Result;
	}

	FPolylineData Polyline;
	if (!BuildPolyline(WorldPoints, bClosed, Polyline))
	{
		Result.Error = TEXT("Contour contains a non-finite point.");
		return Result;
	}

	const double BoxLength = Settings.BoxLengthMeters * CentimetersPerMeter;
	const double Step = (Settings.BoxLengthMeters + Settings.GapLengthMeters) * CentimetersPerMeter;
	Result.PolylineLengthMeters = Polyline.TotalLength / CentimetersPerMeter;
	Result.bIsValid = true;
	if (Polyline.TotalLength + KINDA_SMALL_NUMBER < BoxLength)
	{
		return Result;
	}

	const int64 ExpectedCount = FMath::FloorToInt64((Polyline.TotalLength - BoxLength) / Step) + 1;
	if (ExpectedCount > Settings.MaxInstances)
	{
		Result.bIsValid = false;
		Result.Error = FString::Printf(
			TEXT("Contour would create %lld instances, exceeding the limit of %d."),
			ExpectedCount,
			Settings.MaxInstances);
		return Result;
	}

	const double CenterZ = (ContourWorldZMeters + Settings.ZOffsetMeters + Settings.BoxHeightMeters * 0.5) * CentimetersPerMeter;
	const FVector Scale(
		Settings.BoxLengthMeters,
		Settings.BoxThicknessMeters,
		Settings.BoxHeightMeters);
	for (int64 Index = 0; Index < ExpectedCount; ++Index)
	{
		const double StartDistance = Index * Step;
		const double CenterDistance = StartDistance + BoxLength * 0.5;
		const double EndDistance = StartDistance + BoxLength;
		FVector Start;
		FVector Center;
		FVector End;
		if (!SamplePolyline(Polyline, StartDistance, Start) ||
			!SamplePolyline(Polyline, CenterDistance, Center) ||
			!SamplePolyline(Polyline, EndDistance, End))
		{
			continue;
		}
		const FVector2D Tangent(End.X - Start.X, End.Y - Start.Y);
		if (Tangent.IsNearlyZero())
		{
			continue;
		}
		const double YawDegrees = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
		Center.Z = CenterZ;
		Result.Transforms.Emplace(FRotator(0.0, YawDegrees, 0.0), Center, Scale);
	}
	return Result;
}
