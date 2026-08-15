#include "HeightRangeGenerator.h"

#include "HeightZoneGenerator.h"

namespace
{
struct FUniqueBoundary
{
	double HeightMeters = 0.0;
	FLinearColor Color = FLinearColor::White;
};
}

FHeightRangeDefinition FHeightRangeGenerator::Normalize(const FHeightRangeDefinition& Range)
{
	FHeightRangeDefinition Result = Range;
	Result.MinHeightMeters = FMath::Min(Range.MinHeightMeters, Range.MaxHeightMeters);
	Result.MaxHeightMeters = FMath::Max(Range.MinHeightMeters, Range.MaxHeightMeters);
	return Result;
}

void FHeightRangeGenerator::NormalizeAndSort(TArray<FHeightRangeDefinition>& Ranges)
{
	for (FHeightRangeDefinition& Range : Ranges)
	{
		Range = Normalize(Range);
	}
	Ranges.Sort([](const FHeightRangeDefinition& A, const FHeightRangeDefinition& B)
	{
		if (A.MinHeightMeters == B.MinHeightMeters)
		{
			return A.MaxHeightMeters < B.MaxHeightMeters;
		}
		return A.MinHeightMeters < B.MinHeightMeters;
	});
}

FHeightRangeValidationResult FHeightRangeGenerator::FindConflict(
	const FHeightRangeDefinition& Candidate,
	const TArray<FHeightRangeDefinition>& ExistingRanges)
{
	const FHeightRangeDefinition NormalizedCandidate = Normalize(Candidate);
	for (int32 Index = 0; Index < ExistingRanges.Num(); ++Index)
	{
		const FHeightRangeDefinition Existing = Normalize(ExistingRanges[Index]);
		if (FMath::IsNearlyEqual(NormalizedCandidate.MinHeightMeters, Existing.MinHeightMeters) &&
			FMath::IsNearlyEqual(NormalizedCandidate.MaxHeightMeters, Existing.MaxHeightMeters))
		{
			return {EHeightRangeConflict::Duplicate, Index};
		}

		const bool bOverlap =
			NormalizedCandidate.MinHeightMeters < Existing.MaxHeightMeters &&
			NormalizedCandidate.MaxHeightMeters > Existing.MinHeightMeters;
		if (bOverlap)
		{
			return {EHeightRangeConflict::Overlap, Index};
		}
	}
	return {};
}

bool FHeightRangeGenerator::ContainsHeight(
	const FHeightRangeDefinition& Range,
	double HeightMeters,
	bool bIncludeMaximum)
{
	return HeightMeters >= Range.MinHeightMeters &&
		(bIncludeMaximum
			? HeightMeters <= Range.MaxHeightMeters
			: HeightMeters < Range.MaxHeightMeters);
}

FMultiHeightRangeResult FHeightRangeGenerator::Generate(
	const TArray<float>& HeightMeters,
	int32 Width,
	int32 Height,
	const TArray<FHeightRangeDefinition>& Ranges)
{
	FMultiHeightRangeResult Result;
	Result.Width = Width;
	Result.Height = Height;
	Result.Ranges = Ranges;
	NormalizeAndSort(Result.Ranges);

	if (Width < 2 || Height < 2 || HeightMeters.Num() != Width * Height)
	{
		return Result;
	}

	Result.RangeIndexByPixel.Init(INDEX_NONE, HeightMeters.Num());
	Result.PixelCounts.Init(0, Result.Ranges.Num());

	int32 UppermostEnabledRange = INDEX_NONE;
	for (int32 RangeIndex = 0; RangeIndex < Result.Ranges.Num(); ++RangeIndex)
	{
		if (Result.Ranges[RangeIndex].bEnabled)
		{
			UppermostEnabledRange = RangeIndex;
		}
	}

	for (int32 PixelIndex = 0; PixelIndex < HeightMeters.Num(); ++PixelIndex)
	{
		for (int32 RangeIndex = 0; RangeIndex < Result.Ranges.Num(); ++RangeIndex)
		{
			const FHeightRangeDefinition& Range = Result.Ranges[RangeIndex];
			if (!Range.bEnabled)
			{
				continue;
			}
			if (ContainsHeight(Range, HeightMeters[PixelIndex], RangeIndex == UppermostEnabledRange))
			{
				Result.RangeIndexByPixel[PixelIndex] = RangeIndex;
				++Result.PixelCounts[RangeIndex];
				break;
			}
		}
	}

	TArray<FUniqueBoundary> UniqueBoundaries;
	for (const FHeightRangeDefinition& Range : Result.Ranges)
	{
		if (!Range.bEnabled)
		{
			continue;
		}
		const FUniqueBoundary Boundaries[2] =
		{
			{Range.MinHeightMeters, Range.Color},
			{Range.MaxHeightMeters, Range.Color}
		};
		for (const FUniqueBoundary& Boundary : Boundaries)
		{
			FUniqueBoundary* Existing = UniqueBoundaries.FindByPredicate(
				[&Boundary](const FUniqueBoundary& Item)
				{
					return FMath::IsNearlyEqual(Item.HeightMeters, Boundary.HeightMeters);
				});
			if (Existing)
			{
				// Sorted ranges make the later definition the upper neighbor, whose
				// color also owns the shared [Min, Max) boundary.
				Existing->Color = Boundary.Color;
			}
			else
			{
				UniqueBoundaries.Add(Boundary);
			}
		}
	}

	for (const FUniqueBoundary& Boundary : UniqueBoundaries)
	{
		FHeightZoneResult BoundaryResult = FHeightZoneGenerator::Generate(
			HeightMeters,
			Width,
			Height,
			Boundary.HeightMeters,
			EHeightZoneMode::ContourOnly);
		for (FHeightContour& Contour : BoundaryResult.Contours)
		{
			Contour.Color = Boundary.Color;
			Result.Contours.Add(MoveTemp(Contour));
		}
	}

	return Result;
}

