// Fill out your copyright notice in the Description page of Project Settings.


#include "VomitSpawner.h"

#include "NavigationSystem.h"
#include "Bugnauts/ProjectileSystem/ObjectPoolSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UVomitSpawner::UVomitSpawner()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//setup default values
	AmountOfProjectilesInAWave = 1;
	NumberOfWaves = 20;
	DurationBetweenWaves = 0.4f;
	MaximumRadiusAroundPlayer = 500;
}


// Called when the game starts
void UVomitSpawner::BeginPlay()
{
	Super::BeginPlay();

	//get object pool
	ObjectPool = GetWorld()->GetGameInstance()->GetSubsystem<UObjectPoolSubsystem>();
}


// Called every frame
void UVomitSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UVomitSpawner::StartFiring()
{
	//if something already firing, stop it and also notify whatever triggered it that it has stopped
	if (bCurrentlyShooting)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		FinishedDelegate.Broadcast();
	}

	//start firing sequence
	bCurrentlyShooting = true;
	WaveCounter = 0;
	Fire();
}

void UVomitSpawner::StartNextTimer()
{
	//has it ran all waves
	if (WaveCounter >= NumberOfWaves)
	{
		bCurrentlyShooting = false;
		FinishedDelegate.Broadcast();
		return;
	}

	//start next wave
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UVomitSpawner::Fire, DurationBetweenWaves);
}

void UVomitSpawner::Fire()
{
	//actually fire the amount of projectiles neccesary in the wave
	for (int i = 0; i < AmountOfProjectilesInAWave; i++)
	{
		if(ABasicProjectile* Projectile = CreateProjectile())
		{
			Projectile->Fire(FVector::UpVector, ProjData, 0);
		}
	}

	//set next wave
	WaveCounter++;
	StartNextTimer();
}

ABasicProjectile* UVomitSpawner::CreateProjectile()
{

	//find the ground location that the player is on 
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceType);
	bool bDidHit = GetWorld()->LineTraceSingleByChannel(HitResult, Player->GetActorLocation(),
														Player->GetActorLocation() + FVector::DownVector * 10000,
														CollisionChannel, Params);
	//the player is over the void... somehow
	if(!bDidHit)
	{
		return nullptr;
	}

	//find a random location on the map that is walkable around the player 
	FNavLocation NavLocation;
	if(UNavigationSystemV1* Nav = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem()))
	{
		bool bDidFind = Nav->GetRandomReachablePointInRadius(HitResult.Location, MaximumRadiusAroundPlayer, NavLocation);
		if(!bDidFind) return nullptr;
	}

	//actually create projectile and setup the projectile
	ABasicProjectile* Projectile = ObjectPool->GetBulletFromPool();
	Projectile->SetActorLocation(GetComponentLocation());
	if (USphereComponent* SphereCollision = Projectile->GetComponentByClass<USphereComponent>())
	{
		SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	Projectile->bFiredByEnemy = true;
	Projectile->ApplyProjectileData(ProjData, 0.6f);

	//create a spline 
	AActor* SplineActor = GetWorld()->SpawnActor(AActor::StaticClass());
	
	USplineComponent* Spline = NewObject<USplineComponent>(SplineActor);
	Spline->RegisterComponent();

	//setup spline with the correct values for the projectile to travel
	FVector StartPoint = GetComponentLocation();
	FVector TargetPoint = NavLocation.Location;
	FVector MidPoint = (StartPoint + TargetPoint) / 2;
	MidPoint.Z = StartPoint.Z + AdditionalZHeight;

	Spline->SetLocationAtSplinePoint(0, StartPoint, ESplineCoordinateSpace::World, false);
	Spline->SetLocationAtSplinePoint(1, TargetPoint, ESplineCoordinateSpace::World, false);

	Spline->SetTangentAtSplinePoint(0, (MidPoint-StartPoint) * 2, ESplineCoordinateSpace::Local, false);
	Spline->SetTangentAtSplinePoint(1, (TargetPoint-MidPoint) * 2, ESplineCoordinateSpace::Local, false);
	Spline->UpdateSpline();

	//setup indicator for where projectiels will land
	AActor* Indicator = GetWorld()->SpawnActor(IndicatorActorClass, &TargetPoint, &FRotator::ZeroRotator);
	Projectile->SetProjectileVomitSpline(Spline);

	Projectile->SplineActor = SplineActor;
	Projectile->Indicator = Indicator;
	
	
	return Projectile;
}
