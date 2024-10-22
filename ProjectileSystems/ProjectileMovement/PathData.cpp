// Fill out your copyright notice in the Description page of Project Settings.


#include "PathData.h"

const FInterpCurvePointVector UPathData::DummyPointPosition(0.0f, FVector::ZeroVector, FVector::ForwardVector, FVector::ForwardVector, CIM_Constant);


UPathData::UPathData()
{
	// FPathPoint StartingPoint(0, FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector);
	// PathPoints.Add(StartingPoint);

	Spline.Points.Emplace(0, FVector::ZeroVector, FVector(100, 100, 0), FVector(100, 100 ,0), CIM_CurveUser);
	
}

float UPathData::GetSegmentLength(int32 SegmentIndex, float DistanceParam)
{
	//fun math that I only understand 50% if I'm being honest
	//but not enough time :c
	const int32 NumberofPoints = Spline.Points.Num();
	const int32 LastPoint = NumberofPoints - 1;
	check(DistanceParam >= 0.0f && DistanceParam <= 1.0f);

	//some fucking black magic math
	struct FLegendreGaussCoefficient
	{
		float Abscissa;
		float Weight;
	};

	static const FLegendreGaussCoefficient LegendreGaussCoefficients[] =
	{
		{ 0.0f, 0.5688889f },
		{ -0.5384693f, 0.47862867f },
		{ 0.5384693f, 0.47862867f },
		{ -0.90617985f, 0.23692688f },
		{ 0.90617985f, 0.23692688f }
	};

	const FInterpCurvePoint<FVector>& StartPoint = Spline.Points[SegmentIndex];
	const FInterpCurvePoint<FVector>& EndPoint = Spline.Points[SegmentIndex == LastPoint ? 0 : SegmentIndex + 1];

	const auto& P0 = StartPoint.OutVal;
	const auto& T0 = StartPoint.LeaveTangent;
	const auto& P1 = EndPoint.OutVal;
	const auto& T1 = EndPoint.ArriveTangent;

	if (StartPoint.InterpMode == CIM_Linear)
	{
		return ((P1 - P0) * 1).Size() * DistanceParam;
	}
	else if (StartPoint.InterpMode == CIM_Constant)
	{
		// Special case: constant interpolation acts like distance = 0 for all p in [0, 1[ but for p == 1, the distance returned is the linear distance between start and end
		return DistanceParam == 1.f ? ((P1 - P0) * 1).Size() : 0.0f;
	}

	const FVector Coeff1 = ((P0 - P1) * 2.0f + T0 + T1) * 3.0f;
	const FVector Coeff2 = (P1 - P0) * 6.0f - T0 * 4.0f - T1 * 2.0f;
	const FVector Coeff3 = T0;

	const float HalfParam = DistanceParam * 0.5f;

	float Length = 0.0f;
	for (const auto& LegendreGaussCoefficient : LegendreGaussCoefficients)
	{
		// Calculate derivative at each Legendre-Gauss sample, and perform a weighted sum
		const float Alpha = HalfParam * (1.0f + LegendreGaussCoefficient.Abscissa);
		const FVector Derivative = ((Coeff1 * Alpha + Coeff2) * Alpha + Coeff3) * 1;
		Length += Derivative.Size() * LegendreGaussCoefficient.Weight;
	}
	Length *= HalfParam;

	return Length;
}

void UPathData::UpdateSpline()
{
	const int32 NumberOfPoints  = Spline.Points.Num();

	Spline.AutoSetTangents();

	//Check to make sure spline points are ascending
	for(int32 Index = 1; Index < NumberOfPoints; Index++)
	{
		ensureAlways(Spline.Points[Index - 1].InVal < Spline.Points[Index].InVal);
	}

	const int32 NumSegments = FMath::Max(0, NumberOfPoints - 1);

	const int32 DistanceSteps = 10;
	
	DistanceLookupTable.Points.Reset(NumSegments * DistanceSteps + 1);
	float AccumulatedLength = 0.0f;
	for(int32 SegmentIndex = 0; SegmentIndex < NumSegments; ++SegmentIndex)
	{
		for(int32 Step = 0; Step < DistanceSteps; ++Step)
		{
			const float DistanceParam = static_cast<float>(Step) / DistanceSteps;
			const float SegmentLength = (Step == 0) ? 0.0f : GetSegmentLength(SegmentIndex, DistanceParam);

			DistanceLookupTable.Points.Emplace(SegmentLength + AccumulatedLength, SegmentIndex + DistanceParam, 0.0f, 0.0f, CIM_Linear);
			
		}
		AccumulatedLength += GetSegmentLength(SegmentIndex, 1.0f);
	}
	DistanceLookupTable.Points.Emplace(AccumulatedLength, static_cast<float>(NumSegments), 0.0f, 0.0f, CIM_Linear);
	TotalDistance = AccumulatedLength;
}

FVector UPathData::GetLocationAtInputKey(const float Param) const
{

	FVector Location = Spline.Eval(Param, FVector::ZeroVector);
	// Location = Origin.TransformPosition(Location);
	
	return Spline.Eval(Param, FVector::ZeroVector);
}

FVector UPathData::GetLocationAtDistance(const float Distance) const
{
	const float Param = DistanceLookupTable.Eval(Distance, 0.0f);
	return GetLocationAtInputKey(Param);
}
//
// void UPathData::AddPointToEnd(FPathPoint PathPoint)
// {
// 	PathPoints.Add(PathPoint);
// 	
// }
//
// void UPathData::RemovePoint(uint32 Index)
// {
// 	if(Index >= PathPoints.Num() && Index == 0)
// 	{
// 		PathPoints.RemoveAt(Index);
// 	}
// }
//
// FPathPoint& UPathData::GetPoint(uint32 Index)
// {
// 	return PathPoints[Index];
// }
//
// uint32 UPathData::GetLength()
// {
// 	return PathPoints.Num();
// }


