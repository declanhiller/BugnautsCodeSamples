// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "BasicProjectileMovementComponent.generated.h"

class UPathData;
DECLARE_DELEGATE(FOnLifetimeOver)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGNAUTS_API UBasicProjectileMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBasicProjectileMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FVector Direction;
	
	UPROPERTY(EditAnywhere)
	float CurrentLifetime;

	//when did bullet start flying
	float FlightPathStartTime;

	//lifetime of bullet on previous frame
	UPROPERTY()
	float PreviousLifetime; //for calculating 

	UPROPERTY()
	float TotalLifetime;

	//how many cm has the bullet traveled
	UPROPERTY()
	float CurrentDistance;

	//direction that bullet is moving
	UPROPERTY()
	FVector DirectionBulletIsMoving;
	

public:
	//move the bullet along the path
	void MoveAlongPath(float DeltaTime);
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	                           
	void FireInDirection(const FVector& DirectionInput, const float SpeedInput, const float LifetimeInput); //Shoots attached component with these settings

	//cleanup for pool
	void Cleanup();


	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetVelocity() { return Direction * Speed; }
		
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> UpdateComponent; //Component that is being moved

	//broadcast when bullet dies
	FOnLifetimeOver OnLifetimeOverDelegate;

	UPROPERTY(BlueprintReadOnly)
	bool Enabled;

	UPROPERTY(EditAnywhere)
	float Speed;

	//path that bullet should fire
	UPROPERTY(EditAnywhere)
	UPathData* PathData;

	//hardcoded implementation for bees for their specific vomit attack
	UPROPERTY()
	USplineComponent* VomitSpline;

	//how much distance has been travelled on the spline
	UPROPERTY()
	float DistanceTravelledAmongSpline;

	//"starting" transform for path following projectiles  
	UPROPERTY()
	FTransform Origin;

	//does bullet home
	UPROPERTY()
	bool bHoming;

	//how much does bullet home
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HomingMagnitude;

	//homing target
	UPROPERTY()
	USceneComponent* Target;

	//special homing curve that can control how much bullets home depending on how close they are to target
	UPROPERTY()
	UCurveFloat* HomingMagnitudeCurve;
	

	
};
