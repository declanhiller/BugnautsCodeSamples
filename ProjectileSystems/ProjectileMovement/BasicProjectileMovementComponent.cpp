// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicProjectileMovementComponent.h"

#include "NiagaraComponent.h"
#include "Bugnauts/ProjectileSystem/Data/PathData.h"
#include "Bugnauts/ProjectileSystem/Projectiles/BasicProjectile.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UBasicProjectileMovementComponent::UBasicProjectileMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBasicProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UBasicProjectileMovementComponent::MoveAlongPath(float DeltaTime)
{
	
	if(CurrentDistance >= PathData->GetTotalDistanceOfFlightPath())
	{
		Origin.SetLocation(UpdateComponent->GetComponentLocation());
		// Origin.SetRotation(Origin.TransformVector(PathData->GetForwardVectorAtEndOfFlightPath()).Rotation().Quaternion());
		// FlightPathStartTime = UKismetSystemLibrary::GetGameTimeInSeconds(this);
		CurrentDistance = CurrentDistance - PathData->GetTotalDistanceOfFlightPath();
	}
	CurrentDistance += DeltaTime * Speed;
	// CurrentDistance = (UKismetSystemLibrary::GetGameTimeInSeconds(this) - FlightPathStartTime) * Speed;
	const FVector NextLocation = PathData->GetLocationAtDistance(CurrentDistance);
	FVector TestLocation = Origin.TransformPosition(NextLocation);
	// FString msg("Next Location: ");
	// msg.Append(" ");
	// msg.Append(FString::SanitizeFloat(NextLocation.X));
	// msg.Append(" ");
	// msg.Append(FString::SanitizeFloat(NextLocation.Y));
	// msg.Append(" ");
	// msg.Append(FString::SanitizeFloat(NextLocation.Z));
	// msg.Append(FString::SanitizeFloat(CurrentDistance));
	// GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, msg);
	// UpdateComponent->SetWorldLocation(NextLocation, true);
	// FVector WorldLocation = NextLocation + Origin.GetLocation();
	UpdateComponent->SetWorldLocation(TestLocation, true);
}

// Called every frame
void UBasicProjectileMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	
	if(!Enabled) return;
	CurrentLifetime += DeltaTime;

	
	if(Target && bHoming) // moves based on homing target
	{
		FVector CurrentVelocity = Direction * Speed;

		//get homing strength that should be used
		float CurrentHomingMagnitude = HomingMagnitudeCurve ? HomingMagnitudeCurve->GetFloatValue(CurrentLifetime) : HomingMagnitude;


		//calculate using homing strength what current velocity should be
		FVector HomingForce = (Target->GetComponentLocation() - UpdateComponent->GetComponentLocation()).GetSafeNormal() * CurrentHomingMagnitude;
		FVector NextVelocity = CurrentVelocity + (HomingForce * DeltaTime);

		NextVelocity = NextVelocity.GetClampedToMaxSize(Speed);

		Direction = NextVelocity.GetSafeNormal();
		
		// FVector HomingDirection = ((Target->GetComponentLocation() - UpdateComponent->GetComponentLocation()).GetSafeNormal());
		UpdateComponent->AddLocalOffset(NextVelocity * DeltaTime, true);
	}else if(PathData) //move with path following data if there is one
	{
		MoveAlongPath(DeltaTime);
	} else if(VomitSpline) //move along spline if there is one
	{
		
		DistanceTravelledAmongSpline += DeltaTime * Speed;
		FVector NextLocation = VomitSpline->GetLocationAtDistanceAlongSpline(DistanceTravelledAmongSpline, ESplineCoordinateSpace::World);
		UpdateComponent->SetWorldLocation(NextLocation, true);
	}
	else //move normally in a straight line
	{
		UpdateComponent->AddLocalOffset(Direction * Speed * DeltaTime, true);
	}
	
	//figure out lifetime stuff to see if bullet should be returned to code or not
	PreviousLifetime = CurrentLifetime;
	if(CurrentLifetime >= TotalLifetime)
	{
		Enabled = false;
		
		//Shitty line of code but whatever
		if(ABasicProjectile* Projectile = Cast<ABasicProjectile>(GetOwner()))
		{
			// Projectile->DestroyProjectile();

			// Projectile->ProjectileMovementComponent->Enabled = false;
			//
			// if (UStaticMeshComponent* StaticMesh = Projectile->GetComponentByClass<UStaticMeshComponent>())
			// {
			// 	StaticMesh->SetVisibility(false);
			// }
			//
			// if(Projectile->BulletTrailComponent) Projectile->BulletTrailComponent->Activate(false);
			//
			// GetWorld()->GetTimerManager().SetTimer(Projectile->DestructionTimerHandle, Projectile, &ABasicProjectile::DestroyProjectile, 2.0f, false);

			//"destroys" projectile
			Projectile->DestroyProjectile();
			
		} else
		{
			OnLifetimeOverDelegate.ExecuteIfBound();
		}
	}
}

void UBasicProjectileMovementComponent::FireInDirection(const FVector& DirectionInput, const float SpeedInput,
	const float LifetimeInput)
{
	this->Direction = DirectionInput;
	// this->Speed = SpeedInput;
	CurrentLifetime = 0;
	PreviousLifetime = 0;
	TotalLifetime = LifetimeInput;
	FlightPathStartTime = UKismetSystemLibrary::GetGameTimeInSeconds(this);
	Enabled = true;
}

void UBasicProjectileMovementComponent::Cleanup()
{
	Enabled = false;
	VomitSpline = nullptr;
	DistanceTravelledAmongSpline = 0;
	Direction = FVector::UpVector;
	CurrentLifetime = 0;
	FlightPathStartTime = 0;
	PreviousLifetime = 0;
	TotalLifetime = 0;
	CurrentDistance = 0;
	Speed = 0;
	PathData = nullptr;
	Origin = FTransform::Identity;
	bHoming = false;
	HomingMagnitude = 0;
	Target = nullptr;
	HomingMagnitudeCurve = nullptr;
}

