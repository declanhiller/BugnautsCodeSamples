// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Math/TransformCalculus3D.h"
#include "PathData.generated.h"

UENUM(BlueprintType)
enum ERepeatingOrientation { DeadOn, JoinTangentHandles, UserDefined };

USTRUCT(BlueprintType)
struct FRepeatingSettings
{
	GENERATED_BODY()

	//Global Settings
	//TODO actually implement proper repeating settings
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ERepeatingOrientation> RepeatingSettings;

	//TODO  implement proper offset for spiral patterns
	UPROPERTY(EditAnywhere)
	FVector ScaleOffset;
	
	//User Defined Settings
	UPROPERTY(EditAnywhere, meta = (EditCondition = "RepeatingSettings == ERepeatingOrientation::UserDefined"))
	FRotator RotationOffset;
	
};

//point that a bullet would follow
USTRUCT(Blueprintable, BlueprintType)
struct FPathPoint
{
	GENERATED_BODY()

	FPathPoint()
		: KeyIndex(1),
		  Location(FVector::ZeroVector),
		  ArriveTangent(FVector::ZeroVector),
		  LeaveTangent(FVector::ZeroVector)
	{
	}

	FPathPoint(uint32 KeyIndex, const FVector& Location, const FVector& ArriveTangent, const FVector& LeaveTangent)
		: KeyIndex(KeyIndex),
		  Location(Location),
		  ArriveTangent(ArriveTangent),
		  LeaveTangent(LeaveTangent)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 KeyIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ArriveTangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeaveTangent;
};

/**
 * 
 */
UCLASS()
class BUGNAUTS_API UPathData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPathData();


	// void AddPointToEnd(FPathPoint PathPoint);
	//
	// void RemovePoint(uint32 Index);
	//
	// FPathPoint& GetPoint(uint32 Index);
	//
	// uint32 GetLength();

	
	FInterpCurveVector& GetSplinePoints() {return Spline;}

	inline const FInterpCurvePointVector& SafeGetPointPosition(int32 PointIndex)
	{
		const TArray<FInterpCurvePointVector>& Points = Spline.Points;
		const int32 NumPoints = Points.Num();
		if (NumPoints > 0)
		{
			const int32 ClampedIndex = FMath::Clamp(PointIndex, 0, NumPoints - 1);
			return Points[ClampedIndex];
		}
		else
		{
			return DummyPointPosition;
		}
	}

	//for editor purposes for the most part
	float GetSegmentLength(int32 SegmentIndex, float DistanceParam);
	void UpdateSpline();

	//get specific details for flight
	FVector GetLocationAtInputKey(const float Param) const;
	FVector GetLocationAtDistance(const float Distance) const;

	float GetTotalDistanceOfFlightPath() const
	{
		return TotalDistance;
	}

	//used to determine how repeating should be determined
	FVector GetForwardVectorAtEndOfFlightPath()
	{
		const FInterpCurvePointVector& Point = SafeGetPointPosition(Spline.Points.Num() - 1);
		const FVector& Direction = Point.LeaveTangent;
		return Direction;
	}

	//how should path repeat
	UPROPERTY(EditAnywhere)
	FRepeatingSettings RepeatingSettings;

protected:

	//actual curve itself
	UPROPERTY()
	FInterpCurveVector Spline;

	//Lookup table for better runtime
	//The x is the distance, the y is the float index of the point on the spline that needs to travel that long on the spline
	UPROPERTY()
	FInterpCurveFloat DistanceLookupTable;

	//total length of the curve for calculating "t"
	UPROPERTY()
	float TotalDistance;

	//used for safe gets
	static const FInterpCurvePointVector DummyPointPosition;
};
