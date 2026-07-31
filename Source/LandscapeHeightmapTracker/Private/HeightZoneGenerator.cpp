#include "HeightZoneGenerator.h"

namespace
{
struct FContourSegment
{
	FVector2D A;
	FVector2D B;
};

struct FContourNode
{
	FVector2D Position = FVector2D::ZeroVector;
	TArray<int32> EdgeIndices;
};

FVector2D InterpolateEdge(
	const FVector2D& A,
	const FVector2D& B,
	double HeightA,
	double HeightB,
	double Target)
{
	const double Denominator = HeightB - HeightA;
	const double T = FMath::IsNearlyZero(Denominator)
		? 0.5
		: FMath::Clamp((Target - HeightA) / Denominator, 0.0, 1.0);
	return A + (B - A) * T;
}

FIntPoint QuantizePoint(const FVector2D& Point)
{
	// Keep keys inside int32 for Unreal's largest practical Landscape resolutions
	// while still matching endpoints far below a visible pixel tolerance.
	constexpr double Quantization = 100000.0;
	return FIntPoint(
		FMath::RoundToInt(Point.X * Quantization),
		FMath::RoundToInt(Point.Y * Quantization));
}

void AddCaseSegments(
	int32 CaseIndex,
	bool bCenterAbove,
	const FVector2D EdgePoints[4],
	TArray<FContourSegment>& Segments)
{
	auto Add = [&Segments, &EdgePoints](int32 EdgeA, int32 EdgeB)
	{
		if (!EdgePoints[EdgeA].Equals(EdgePoints[EdgeB], SMALL_NUMBER))
		{
			Segments.Add({EdgePoints[EdgeA], EdgePoints[EdgeB]});
		}
	};

	switch (CaseIndex)
	{
	case 1: Add(3, 0); break;
	case 2: Add(0, 1); break;
	case 3: Add(3, 1); break;
	case 4: Add(1, 2); break;
	case 5:
		if (bCenterAbove) { Add(0, 1); Add(2, 3); }
		else { Add(3, 0); Add(1, 2); }
		break;
	case 6: Add(0, 2); break;
	case 7: Add(3, 2); break;
	case 8: Add(2, 3); break;
	case 9: Add(0, 2); break;
	case 10:
		if (bCenterAbove) { Add(0, 3); Add(1, 2); }
		else { Add(0, 1); Add(2, 3); }
		break;
	case 11: Add(1, 2); break;
	case 12: Add(3, 1); break;
	case 13: Add(0, 1); break;
	case 14: Add(3, 0); break;
	default: break;
	}
}

TArray<FHeightContour> JoinSegments(const TArray<FContourSegment>& Segments)
{
	TArray<FContourNode> Nodes;
	TArray<FIntPoint> Edges;
	TMap<FIntPoint, int32> NodeByPoint;

	auto GetNode = [&Nodes, &NodeByPoint](const FVector2D& Position)
	{
		const FIntPoint Key = QuantizePoint(Position);
		if (const int32* Existing = NodeByPoint.Find(Key))
		{
			return *Existing;
		}

		const int32 Index = Nodes.Add({Position, {}});
		NodeByPoint.Add(Key, Index);
		return Index;
	};

	for (const FContourSegment& Segment : Segments)
	{
		const int32 A = GetNode(Segment.A);
		const int32 B = GetNode(Segment.B);
		if (A == B)
		{
			continue;
		}

		const int32 EdgeIndex = Edges.Add(FIntPoint(A, B));
		Nodes[A].EdgeIndices.Add(EdgeIndex);
		Nodes[B].EdgeIndices.Add(EdgeIndex);
	}

	TBitArray<> Visited(false, Edges.Num());
	TArray<FHeightContour> Contours;
	auto Trace = [&Nodes, &Edges, &Visited, &Contours](int32 StartNode, int32 StartEdge)
	{
		FHeightContour Contour;
		int32 CurrentNode = StartNode;
		int32 CurrentEdge = StartEdge;
		Contour.Points.Add(Nodes[CurrentNode].Position);

		while (CurrentEdge != INDEX_NONE && !Visited[CurrentEdge])
		{
			Visited[CurrentEdge] = true;
			const FIntPoint Edge = Edges[CurrentEdge];
			const int32 NextNode = Edge.X == CurrentNode ? Edge.Y : Edge.X;
			Contour.Points.Add(Nodes[NextNode].Position);
			if (NextNode == StartNode)
			{
				Contour.bClosed = true;
				break;
			}

			if (Nodes[NextNode].EdgeIndices.Num() != 2)
			{
				break;
			}

			CurrentNode = NextNode;
			CurrentEdge = INDEX_NONE;
			for (const int32 Candidate : Nodes[CurrentNode].EdgeIndices)
			{
				if (!Visited[Candidate])
				{
					CurrentEdge = Candidate;
					break;
				}
			}
		}

		if (Contour.Points.Num() >= 2)
		{
			Contours.Add(MoveTemp(Contour));
		}
	};

	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); ++NodeIndex)
	{
		if (Nodes[NodeIndex].EdgeIndices.Num() == 2)
		{
			continue;
		}
		for (const int32 EdgeIndex : Nodes[NodeIndex].EdgeIndices)
		{
			if (!Visited[EdgeIndex])
			{
				Trace(NodeIndex, EdgeIndex);
			}
		}
	}

	for (int32 EdgeIndex = 0; EdgeIndex < Edges.Num(); ++EdgeIndex)
	{
		if (!Visited[EdgeIndex])
		{
			Trace(Edges[EdgeIndex].X, EdgeIndex);
		}
	}

	return Contours;
}
}

FHeightZoneResult FHeightZoneGenerator::Generate(
	const TArray<float>& HeightMeters,
	int32 Width,
	int32 Height,
	double TargetHeightMeters,
	EHeightZoneMode Mode)
{
	FHeightZoneResult Result;
	Result.TargetHeightMeters = TargetHeightMeters;
	Result.Width = Width;
	Result.Height = Height;

	if (Width < 2 || Height < 2 || HeightMeters.Num() != Width * Height)
	{
		return Result;
	}

	Result.Mask.SetNumZeroed(Width * Height);
	if (Mode != EHeightZoneMode::ContourOnly)
	{
		for (int32 Index = 0; Index < HeightMeters.Num(); ++Index)
		{
			const bool bSelected = Mode == EHeightZoneMode::Above
				? HeightMeters[Index] >= TargetHeightMeters
				: HeightMeters[Index] <= TargetHeightMeters;
			Result.Mask[Index] = bSelected ? 255 : 0;
		}
	}

	TArray<FContourSegment> Segments;
	for (int32 Y = 0; Y < Height - 1; ++Y)
	{
		for (int32 X = 0; X < Width - 1; ++X)
		{
			const double Values[4] =
			{
				HeightMeters[X + Y * Width],
				HeightMeters[X + 1 + Y * Width],
				HeightMeters[X + 1 + (Y + 1) * Width],
				HeightMeters[X + (Y + 1) * Width]
			};
			const FVector2D Positions[4] =
			{
				FVector2D(X, Y),
				FVector2D(X + 1, Y),
				FVector2D(X + 1, Y + 1),
				FVector2D(X, Y + 1)
			};

			int32 CaseIndex = 0;
			for (int32 Corner = 0; Corner < 4; ++Corner)
			{
				if (Values[Corner] >= TargetHeightMeters)
				{
					CaseIndex |= 1 << Corner;
				}
			}
			if (CaseIndex == 0 || CaseIndex == 15)
			{
				continue;
			}

			FVector2D EdgePoints[4] =
			{
				InterpolateEdge(Positions[0], Positions[1], Values[0], Values[1], TargetHeightMeters),
				InterpolateEdge(Positions[1], Positions[2], Values[1], Values[2], TargetHeightMeters),
				InterpolateEdge(Positions[3], Positions[2], Values[3], Values[2], TargetHeightMeters),
				InterpolateEdge(Positions[0], Positions[3], Values[0], Values[3], TargetHeightMeters)
			};
			const double CenterHeight = (Values[0] + Values[1] + Values[2] + Values[3]) * 0.25;
			AddCaseSegments(CaseIndex, CenterHeight >= TargetHeightMeters, EdgePoints, Segments);
		}
	}

	Result.Contours = JoinSegments(Segments);
	for (FHeightContour& Contour : Result.Contours)
	{
		for (FVector2D& Point : Contour.Points)
		{
			Point.X /= Width - 1;
			Point.Y /= Height - 1;
		}
	}
	return Result;
}
