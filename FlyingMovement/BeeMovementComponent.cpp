// Fill out your copyright notice in the Description page of Project Settings.


#include "BeeMovementComponent.h"

#include "DonNavigationHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UBeeMovementComponent::UBeeMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBeeMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	//Get the 3d Navmesh
	NavigationManager = UDonNavigationHelper::DonNavigationManagerForActor(GetOwner());
	//Not really switching but initializing
	SwitchBeeFlightState(Idle);

	//compute the distance to run speed curve
	DartSpeedAccelerationDistance = ComputeEstimatedDistanceForSpeedCurve(0.05f);
}

void UBeeMovementComponent::TickIdleMovement(float DeltaTime)
{
	//find which direction to bob
	float MovementMultiplier = bIsBobbingDown ? -1 : 1;
	//calculate how much to move for this frame
	FVector BobMovement = FVector::UpVector * MovementMultiplier * BobSpeed * DeltaTime;
	UpdatedComponent->MoveComponent(BobMovement, UpdatedComponent->GetComponentQuat(), true);

	//switch the bob direction if min or max has been reached
	if (bIsBobbingDown)
	{
		//switch to bobbing up
		bIsBobbingDown = !(UpdatedComponent->GetComponentLocation().Z <= CurrentMinBobHeight);
	}
	else
	{
		//switch to bobbing down
		bIsBobbingDown = UpdatedComponent->GetComponentLocation().Z >= CurrentMaxBobHeight;
	}
}

void UBeeMovementComponent::TickDartMovement(float DeltaTime)
{
	//if no more segments left in queue means flight is done
	if (PathQueue.IsEmpty())
	{
		SwitchBeeFlightState(Idle);
		SendCurrentFlightPathResponse(Succeeded);
		return;
	}

	//run actual flying or hesitating depending on state
	switch (CurrentDartState)
	{
	case Hesitate:
		TickDartHesitate(DeltaTime);
		break;
	case Flying:
		TickDartFlying(DeltaTime);
		break;
	}
}

void UBeeMovementComponent::TickDartHesitate(float DeltaTime)
{
	//stand still!
	CurrentDartHesitateTimer += DeltaTime;
	//once time is reached, set state to flying again and also setup the next segment to be ran
	if (CurrentDartHesitateTimer >= TimeToHesitate)
	{
		SetupNextSegmentDartValues();
		CurrentDartState = Flying;
	}
}

void UBeeMovementComponent::TickDartFlying(float DeltaTime)
{
	//no dart speed curve so flight is impossible
	if (!DartSpeedCurve)
	{
		SendCurrentFlightPathResponse(Failed);
		return;
	}
	CurrentSegmentDartModeTimer += DeltaTime;

	//Get direction to next segment
	FVector CurrentTargetLocation = CurrentDartSegment.Get<1>();
	FVector Direction = CurrentTargetLocation - UpdatedComponent->GetComponentLocation();
	Direction.Normalize();

	//Has position been reached yet
	if (UKismetMathLibrary::Vector_Distance(UpdatedComponent->GetComponentLocation(), CurrentTargetLocation) < 5)
	{
		PathQueue.Pop();
		CurrentSegmentIndex++;
		CurrentDartState = Hesitate;
		CurrentDartHesitateTimer = 0;
		return;
	}


	//time variable to use... if timer is bigger than the time to reach max velocity we will keep using the velocity of that graph
	float t = CurrentSegmentDartModeTimer >= TimeToReachMaxVelocity
		          ? TimeToReachMaxVelocity
		          : CurrentSegmentDartModeTimer;

	float RemainingDistance = FVector::Distance(UpdatedComponent->GetComponentLocation(), CurrentTargetLocation);
	//for deaccelerating with same speed curve
	if (RemainingDistance <= DartSpeedAccelerationDistance)
	{
		t = GetTBasedOnCurrentDistanceForSpeedCurve(RemainingDistance, 0.05f);
	}
	float Speed = DartSpeedCurve->GetFloatValue(t);
	// GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::SanitizeFloat(Speed));


	//Fix overshoot if there is any
	FVector ProposedMove = Direction * Speed * DeltaTime;
	FVector RemainingMoveDistance = CurrentTargetLocation - UpdatedComponent->GetComponentLocation();
	if (ProposedMove.Size() > RemainingMoveDistance.Size())
	{
		ProposedMove = RemainingMoveDistance;
	}

	FRotator ProposedRotation = ProposedMove.Rotation();
	if(bFacePlayer)
	{
		APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		FVector Heading = Pawn->GetActorLocation() - GetOwner()->GetActorLocation();
		ProposedRotation = Heading.Rotation();
	}
	
	UpdatedComponent->MoveComponent(ProposedMove,
	                                ProposedRotation, true);
}

void UBeeMovementComponent::TickNormalMovement(float DeltaTime)
{
	//check if path is finished
	if (PathQueue.IsEmpty())
	{
		SwitchBeeFlightState(Idle);
		SendCurrentFlightPathResponse(Succeeded);
		return;
	}


	FVector CurrentTargetLocation = *PathQueue.Peek();
	FVector Direction = CurrentTargetLocation - UpdatedComponent->GetComponentLocation();
	Direction.Normalize();

	//Has position been reached yet
	if (UKismetMathLibrary::Vector_Distance(UpdatedComponent->GetComponentLocation(), CurrentTargetLocation) < 5)
	{
		PathQueue.Pop();
		CurrentSegmentIndex++;
		return;
	}

	//if there is an override speed from a flight request set it to that override speed
	float FlightSpeed = NormalFlySpeed;
	if(FlightSettings.OverrideFlightSpeed > 0)
	{
		FlightSpeed = FlightSettings.OverrideFlightSpeed;
	}
	
	//Fix overshoot if there is any
	FVector ProposedMove = Direction * FlightSpeed * DeltaTime;
	FVector RemainingMoveDistance = CurrentTargetLocation - UpdatedComponent->GetComponentLocation();
	bool bMovedRemainingDistance = false;
	if (ProposedMove.Size() > RemainingMoveDistance.Size())
	{
		ProposedMove = RemainingMoveDistance;
		bMovedRemainingDistance = true;
	}

	//calculate rotation of component
	FRotator ProposedRotation = ProposedMove.Rotation();
	if(bFacePlayer)
	{
		//TODO make a target variable or something that can be passed in through a function

		//calculate rotation to face player
		APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		FVector Heading = Pawn->GetActorLocation() - GetOwner()->GetActorLocation();
		ProposedRotation = Heading.Rotation();
	} else
	{

		//calculate rotation to face direction of flight
		if (FMath::Abs(FVector::DotProduct(FVector::UpVector, ProposedMove.GetSafeNormal())) > 0.6f)
		{
			FVector TempProposedMove = ProposedMove;
			TempProposedMove.Z = 0;
			ProposedRotation = TempProposedMove.Rotation();
		}
	}

	//actually move
	UpdatedComponent->MoveComponent(ProposedMove,
	                                ProposedRotation, true);

	//whether it should move to next segment or not
	if (bMovedRemainingDistance)
	{
		PathQueue.Pop();
		CurrentSegmentIndex++;
	}
}

FVector UBeeMovementComponent::GetSegmentDirection(int32 Index)
{
	TArray<FVector> Path = CurrentPathData.PathSolutionOptimized;
	if (Path.Num() <= 1) return FVector::ZeroVector;
	if (Index < 0 || Index >= Path.Num() - 1) return FVector::ZeroVector;
	FVector Direction = Path[Index + 1] - Path[Index];
	Direction.Normalize();
	return Direction;
}

void UBeeMovementComponent::SetupNextSegmentDartValues()
{
	CurrentDartState = Flying;
	CurrentSegmentDartModeTimer = 0;
	const FVector StartPoint = UpdatedComponent->GetComponentLocation();
	const FVector EndPoint = *PathQueue.Peek();

	CurrentDartSegment = TTuple<FVector, FVector>(StartPoint, EndPoint);
}

void UBeeMovementComponent::SwitchBeeFlightState(EBeeFlightState NewState)
{
	//Handle exiting a state
	switch (CurrentFlightState)
	{
	case Idle:
		break;
	case Normal:
		break;
	case Darting:
		break;
	}

	//Handle state entry
	switch (NewState)
	{
	case Idle:
		SetBobHeightValues();
		break;
	case Normal:
		break;
	case Darting:
		SetupNextSegmentDartValues();
		break;
	}
	CurrentFlightState = NewState;
}


// Called every frame
void UBeeMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//tick the correct movement mode
	switch (CurrentFlightState)
	{
	case Still:
		break;
	case Idle:
		TickIdleMovement(DeltaTime);
		break;
	case Normal:
		TickNormalMovement(DeltaTime);
		break;
	case Darting:
		TickDartMovement(DeltaTime);
	default: ;
	}
}


void UBeeMovementComponent::FlyTo(const FVector InDestination, FOnFinishedFlying OnReachedDestination, FFlightSettings InFlightSettings,
                                  float InAcceptanceRadius, bool bShouldFacePlayer, TEnumAsByte<EBeeFlightMode> ModeToFly)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString("Receive Fly Signal"));
	//Mark previous flight as cancelled
	if (CurrentFlightPathResponse.IsBound()) SendCurrentFlightPathResponse(Aborted);
	CurrentFlightPathResponse = OnReachedDestination;
	Destination = InDestination;

	AcceptanceRadius = InAcceptanceRadius;

	//Setup flight pathfinding task
	FDoNNavigationQueryParams QueryParams;
	QueryParams.QueryTimeout = 3;
	QueryParams.bFlexibleOriginGoal = true;
	FDoNNavigationDebugParams DebugParams;
	DebugParams.DrawDebugVolumes = false;
	DebugParams.VisualizeOptimizedPath = false;

	FDoNNavigationResultHandler ResultHandler;
	ResultHandler.BindDynamic(this, &UBeeMovementComponent::ReceivePathfindingData);
	FDonNavigationDynamicCollisionDelegate DynamicCollisionHandler;

	//queue flight up for when path is calculated
	QueuedFlightMode = ModeToFly;
	bQueuedFacePlayer = bShouldFacePlayer;
	QueuedFlightSettings = InFlightSettings;

	GetWorld()->GetTimerManager().SetTimer(FailTimerHandle, this, &UBeeMovementComponent::FailFlight, 1.f);

	NavigationManager->SchedulePathfindingTask(GetOwner(), InDestination, QueryParams, DebugParams, ResultHandler,
	                                           DynamicCollisionHandler);
}

void UBeeMovementComponent::DashAwayFromTarget(FVector DashAwayTarget, FOnFinishedFlying OnReachedDestination,
                                               float TargetDistanceAwayFromTarget,
                                               TEnumAsByte<EBeeFlightMode> ModeToFly)
{
	//calculate target position away from passed in target arguement
	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	if(FVector::Distance(CurrentLocation, DashAwayTarget) >= TargetDistanceAwayFromTarget)
	{
		OnReachedDestination.ExecuteIfBound(Succeeded);
		return;
	}

	FVector HeadingToMoveAway = CurrentLocation - DashAwayTarget;
	HeadingToMoveAway.Normalize();
	if(HeadingToMoveAway.Z < 0)
	{
		HeadingToMoveAway.Z = 0;
	}
	FVector ProposedDestination = DashAwayTarget + HeadingToMoveAway * TargetDistanceAwayFromTarget;
	FlyTo(ProposedDestination, OnReachedDestination, FFlightSettings(), 20, DartMode);
}

void UBeeMovementComponent::Abort()
{
	
	if (CurrentFlightPathResponse.IsBound()) SendCurrentFlightPathResponse(Aborted);
	CurrentFlightState = Still;
}


void UBeeMovementComponent::ReceivePathfindingData(const FDoNNavigationQueryData& Data)
{
	GetWorld()->GetTimerManager().ClearTimer(FailTimerHandle);
	//Mark goal as failed
	if (Data.QueryStatus != EDonNavigationQueryStatus::Success)
	{
		SendCurrentFlightPathResponse(Failed);
		return;
	}

	TArray<FVector> Path = Data.PathSolutionOptimized;

	for (FVector Point : Path)
	{
		PathQueue.Enqueue(Point);
	}

	CurrentPathData = Data;

	CurrentPathData.PathSolutionOptimized.Insert(UpdatedComponent->GetComponentLocation(), 0);

	CurrentSegmentIndex = 0;

	CurrentRequestFlightMode = QueuedFlightMode;
	bFacePlayer = bQueuedFacePlayer;
	FlightSettings = QueuedFlightSettings;

	switch (CurrentRequestFlightMode)
	{
	case NormalMode:
		SwitchBeeFlightState(Normal);
		break;
	case DartMode:
		SwitchBeeFlightState(Darting);
		break;
	}
}

void UBeeMovementComponent::FailFlight()
{
	SendCurrentFlightPathResponse(Failed);
}

void UBeeMovementComponent::SetBobHeightValues()
{
	CurrentMinBobHeight = UpdatedComponent->GetComponentLocation().Z - BobDistance / 2;
	CurrentMaxBobHeight = UpdatedComponent->GetComponentLocation().Z + BobDistance / 2;
}

void UBeeMovementComponent::SendCurrentFlightPathResponse(EFlightResult Result)
{
	CurrentFlightPathResponse.ExecuteIfBound(Result);
	CurrentFlightPathResponse.Unbind();
}

float UBeeMovementComponent::ComputeEstimatedDistanceForSpeedCurve(float TimeStepForRiemannSum)
{
	if (!DartSpeedCurve) return 0;
	float CurrentStartTimeInterval = 0;
	float CurrentEndTimeInterval = TimeStepForRiemannSum;
	float Distance = 0;
	//run a simple riemman sum to calculate distances from a speed(velocity) curve to figure out how much to move component when accelerating
	while (true)
	{
		float Width = TimeStepForRiemannSum;
		float t = (CurrentEndTimeInterval + CurrentStartTimeInterval) / 2;
		float Height = DartSpeedCurve->GetFloatValue(t);
		float Area = Width * Height;
		Distance += Area;
		CurrentStartTimeInterval += TimeStepForRiemannSum;
		CurrentEndTimeInterval += TimeStepForRiemannSum;
		if (CurrentStartTimeInterval >= TimeToReachMaxVelocity) break;

		if (CurrentEndTimeInterval > TimeToReachMaxVelocity)
		{
			CurrentEndTimeInterval = TimeToReachMaxVelocity;
		}
	}
	return Distance;
}

float UBeeMovementComponent::GetTBasedOnCurrentDistanceForSpeedCurve(float Distance, float TimeStepForRiemannSum)
{
	if (!DartSpeedCurve) return 0;
	float CurrentStartTimeInterval = 0;
	float CurrentEndTimeInterval = TimeStepForRiemannSum;
	float TrackedDistance = 0;
	int attemptNumber = 0; //prevent timeouts and freezes

	//finding the "x" based on the integral of y basically
	while (true)
	{
		attemptNumber++;
		float Width = CurrentEndTimeInterval - CurrentStartTimeInterval;
		float t = (CurrentEndTimeInterval + CurrentStartTimeInterval) / 2;
		float Height = DartSpeedCurve->GetFloatValue(t);
		float Area = Width * Height;
		TrackedDistance += Area;
		if (TrackedDistance >= Distance)
		{
			return CurrentEndTimeInterval;
		}
		if (attemptNumber > 300) break;
		CurrentStartTimeInterval += TimeStepForRiemannSum;
		CurrentEndTimeInterval += TimeStepForRiemannSum;
	}
	return 0;
}
