// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPool.h"
#include "ProjectileMovement/BasicProjectileMovementComponent.h"
#include "Projectiles/BasicProjectile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 *
 */
UCLASS(Abstract, Blueprintable)
class BUGNAUTS_API UObjectPoolSubsystem : public UGameInstanceSubsystem, public IObjectPool
{
private:
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//setup the pool
	void CreatePool();

	//gets a basic bullet from pool
	UFUNCTION(BlueprintCallable)
		virtual ABasicProjectile* GetBulletFromPool() override;

	//return bullet to pool
	UFUNCTION(BlueprintCallable)
		virtual void ReturnBulletToPool(ABasicProjectile* Bullet) override;

	//removes from active bullets but doesn't return to pool
	UFUNCTION(BlueprintCallable)
		virtual void PseudoReturnBulletToPool(ABasicProjectile* Bullet);

protected:
	UPROPERTY(EditAnywhere)
		int32 PoolSize;

	UPROPERTY(EditAnywhere)
		TSubclassOf<ABasicProjectile> BulletClass;

public:
	UPROPERTY()
		TArray<ABasicProjectile*> ActiveProjectiles;

private:
	UPROPERTY()
		TArray<ABasicProjectile*> ObjectPool;

	//Connect listeners so that projectiles can be returned back to object pool without knowing about the object pool
	void SetupDelegates(UBasicProjectileMovementComponent* BulletComponent, ABasicProjectile* Bullet);



};
