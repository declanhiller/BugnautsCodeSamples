// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DonNavigationManager.h"
#include "GameFramework/MovementComponent.h"
#include "BeeMovementComponent.generated.h"


//For AI tasks and async functionality
UENUM(BlueprintType)
enum EFlightResult { Failed, Aborted, Succeeded };


//Flight Mode and Flight State are slightly different and are mostly here due to a shift in architecture but mode was not fully removed
UENUM(BlueprintType, Blueprintable)
enum EBeeFlightMode {NormalMode, DartMode};

UENUM(BlueprintType)
enum EBeeFlightState { Still, Idle, Normal, Darting };

//State in the dart state itself
UENUM(BlueprintType)
enum EBeeDartState { Hesitate, Flying };

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFinishedFlying, EFlightResult, Result);

//struct that could be inputed that controlled how the bee would fly
USTRUCT(BlueprintType)
struct FFlightSettings
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	bool bStayLevel = false;

	UPROPERTY(BlueprintReadWrite)
	float OverrideFlightSpeed = -1;
};

//todo might want to separate this into a ai controller component and a component that goes on the actor depending but bees seem like a one off so this is probably fine
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGNAUTS_API UBeeMovementComponent : public UMovementComponent 
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBeeMovementComponent();

	//Stored delegate to execute when reached destination
	FOnFinishedFlying CurrentFlightPathResponse;

	FVector Destination;

	//To schedule pathfinding tasks
	ADonNavigationManager* NavigationManager;

	UPROPERTY(Category = "Normal Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits="cm/s"))
	float NormalFlySpeed;
	
	UPROPERTY(Category = "Normale Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits="cm/s^2"))
	float NormalFlyAcceleration;

	
	UPROPERTY(Category = "Dart Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits="cm/s^2"))
	UCurveFloat* DartSpeedCurve;
	
	UPROPERTY(Category = "Dart Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits="cm/s^2"))
	float TimeToReachMaxVelocity;

	UPROPERTY(Category = "Dart Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits="cm/s^2"))
	float TimeToHesitate;

	UPROPERTY(Category = "Idle Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BobDistance;

	UPROPERTY(Category = "Idle Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float BobSpeed;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Running Idle Movement (bobbing most of the time)
	virtual void TickIdleMovement(float DeltaTime);

	//Calls Hesitate and Flying specifically
	virtual void TickDartMovement(float DeltaTime);
	//Basically a timer that will make the bee wait a bit on each point
	virtual void TickDartHesitate(float DeltaTime);
	//Actually moving the bee when it's darting
	virtual void TickDartFlying(float DeltaTime);


	//Ticking normal movement which is just a constant speed along a path
	virtual void TickNormalMovement(float DeltaTime);

	//Get the direction of the segment at the index in the current loaded path
	FVector GetSegmentDirection(int32 Index);

	
	void SetupNextSegmentDartValues();

	//switch the state that the bee is in
	virtual void SwitchBeeFlightState(EBeeFlightState NewState);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//What to trigger pathfinding movement
	UFUNCTION(BlueprintCallable)
	virtual void FlyTo(const FVector InDestination, FOnFinishedFlying OnReachedDestination, FFlightSettings InFlightSettings, float InAcceptanceRadius = 30, bool bShouldFacePlayer = false, TEnumAsByte<EBeeFlightMode> ModeToFly = NormalMode);


	//Special node that setups the bee to move away from a specific target and then actually moves them away
	UFUNCTION(BlueprintCallable)
	virtual void DashAwayFromTarget(FVector DashAwayTarget, FOnFinishedFlying OnReachedDestination, float TargetDistanceAwayFromTarget = 300, TEnumAsByte<
	                                EBeeFlightMode> ModeToFly = DartMode);

	//Stops bee but allows bee to be resumed by just setting the state to a movement state
	UFUNCTION(BlueprintCallable)
	virtual void StunBee()
	{
		CurrentFlightState = Still;
	}

	//Aborts all current movement and resets bee state to base
	UFUNCTION(BlueprintCallable)
	void Abort();


private:
	//Binded to a delegate that is send into the pathfinding algorithm, this is how we get our pathfinding data
	UFUNCTION()
	void ReceivePathfindingData(const FDoNNavigationQueryData& Data);

	//Pathfinding data for flight
	FDoNNavigationQueryData CurrentPathData;

	//Generated Path to take
	TQueue<FVector> PathQueue;

	EBeeFlightState CurrentFlightState;

	//Timeout handle so that bee doesn't get frozen
	FTimerHandle FailTimerHandle;

	//Manually fail a flight that is currently in progress
	void FailFlight();

	//How close is it considered reaching a destination
	float AcceptanceRadius;

	//Current segment of the path that the bee is on
	int32 CurrentSegmentIndex;

	
	float Timer;

	//current position on segment as a ratio
	float CurrentAlphaValueForSegment;

	//Tracks the amount of time that has been on segment
	float CurrentSegmentDartModeTimer;
	//Tracks the amount of time that bee has hesitated
	float CurrentDartHesitateTimer;
	//tracks which state that they are in while in the dart state
	EBeeDartState CurrentDartState;
	//stores the current segment with first tmeplate being start and second being end
	TTuple<FVector, FVector> CurrentDartSegment;
	//distance that the movement comp will accelerate
	float DartSpeedAccelerationDistance;

	//Minimum distance that movement comp will bob
	float CurrentMinBobHeight;

	//Max distance that movement comp will bob
	float CurrentMaxBobHeight;

	//Is specifically bobbing down so we know specific direction that bob is occuring
	bool bIsBobbingDown;

	//setup currentMinBobDistance and currentMaxBobDistance values
	void SetBobHeightValues();

	//Flight mode that is currently requested
	EBeeFlightMode CurrentRequestFlightMode;

	//Flight mode that is next up once current one is finished
	EBeeFlightMode QueuedFlightMode;

	//queued version of whether bee should face player when moving
	bool bQueuedFacePlayer;
	//whether bee should face player when moving
	bool bFacePlayer;

	//Settings for queued flight
	FFlightSettings QueuedFlightSettings;
	//Settings for current flight
	FFlightSettings FlightSettings;
	
	//broadcast the flight result of the current flight
	void SendCurrentFlightPathResponse(EFlightResult Result);

	//"Fancy" integral estimation to determine when and how much to use the speed curve
	float ComputeEstimatedDistanceForSpeedCurve(float TimeStepForRiemannSum);
	float GetTBasedOnCurrentDistanceForSpeedCurve(float Distance, float TimeStepForRiemannSum);

	
};
