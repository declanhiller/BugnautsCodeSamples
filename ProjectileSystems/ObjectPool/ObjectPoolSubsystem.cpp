// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPoolSubsystem.h"

#include "NiagaraComponent.h"

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

}

void UObjectPoolSubsystem::CreatePool()
{
	UE_LOG(LogTemp, Warning, TEXT("Start Initializing Object Pool"));

	// Put all projectiles into the object pool
	for (int i = 0; i < PoolSize; i++)
	{
		UWorld* World = GetWorld();
		ABasicProjectile* Projectile = World->SpawnActor<ABasicProjectile>(BulletClass, FVector(0, 0, 0), FRotator::ZeroRotator);
		UBasicProjectileMovementComponent* BulletComponent = Projectile->GetComponentByClass<UBasicProjectileMovementComponent>();
		SetupDelegates(BulletComponent, Projectile);
		ReturnBulletToPool(Projectile);
	}

	UE_LOG(LogTemp, Warning, TEXT("Successfully Initialized Object Pool Subsystem"))
}

ABasicProjectile* UObjectPoolSubsystem::GetBulletFromPool()
{
	UE_LOG(LogTemp, Warning, TEXT("Get bullet from pool"));
	// UE_LOG(LogTemp, Warning, TEXT("Number of Objects currently in Pool is %d"), num);
	ABasicProjectile* Bullet;

	if (ObjectPool.Num() == 0) // if no more projectiles in the pool then create a new one
	{
		Bullet = GetWorld()->SpawnActor<ABasicProjectile>(BulletClass, FVector(0, 0, 0), FRotator::ZeroRotator);
		if (Bullet->BulletTrailComponent) Bullet->BulletTrailComponent->Activate(true);
		UBasicProjectileMovementComponent* BulletComponent = Bullet->GetComponentByClass<UBasicProjectileMovementComponent>();
		SetupDelegates(BulletComponent, Bullet);
	}
	else
	{
		// Bullet = ObjectPool[0];
		// ObjectPool.RemoveAt(0);

		//get bullet and setup basic attributes
		Bullet = ObjectPool.Pop();
		Bullet->SetActorHiddenInGame(false);
		Bullet->SetActorTickEnabled(true);
		Bullet->SetActorEnableCollision(true);
		// if(Bullet->BulletTrailComponent) Bullet->BulletTrailComponent->Activate(true);

	}
	//add into active projectiles
	ActiveProjectiles.Add(Bullet);

	// Bullet = GetWorld()->SpawnActor<ABasicProjectile>(BulletClass, FVector(0, 0, 0), FRotator::ZeroRotator);
	// UBasicProjectileMovementComponent* BulletComponent = Bullet->GetComponentByClass<UBasicProjectileMovementComponent>();
	// SetupDelegates(BulletComponent, Bullet);
	return Bullet;
}

void UObjectPoolSubsystem::ReturnBulletToPool(ABasicProjectile* Bullet)
{
	//return bullet and tear down
	if (UStaticMeshComponent* StaticMesh = Bullet->GetComponentByClass<UStaticMeshComponent>())
	{
		StaticMesh->SetVisibility(true);
	}
	Bullet->SetActorHiddenInGame(true);
	Bullet->SetActorTickEnabled(false);
	Bullet->SetActorEnableCollision(false);
	ObjectPool.Add(Bullet);
}

void UObjectPoolSubsystem::PseudoReturnBulletToPool(ABasicProjectile* Bullet)
{
	ActiveProjectiles.Remove(Bullet);
}

void UObjectPoolSubsystem::SetupDelegates(UBasicProjectileMovementComponent* BulletComponent, ABasicProjectile* Bullet)
{
	Bullet->OnReturnToPoolDelegate.AddLambda([this](ABasicProjectile* Proj) {ReturnBulletToPool(Proj); });
	// BulletComponent->OnLifetimeOverDelegate.BindWeakLambda(this, [this, Bullet]()
	// {
	// 	ReturnBulletToPool(Bullet);
	// });
}
