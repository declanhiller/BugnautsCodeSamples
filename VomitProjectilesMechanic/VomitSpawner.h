// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bugnauts/ProjectileSystem/ObjectPool.h"
#include "Bugnauts/ProjectileSystem/ObjectPoolSubsystem.h"
#include "Components/SceneComponent.h"
#include "VomitSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishFiring);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGNAUTS_API UVomitSpawner : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVomitSpawner();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	UObjectPoolSubsystem* ObjectPool;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//trigger firing
	UFUNCTION(BlueprintCallable)
	void StartFiring();

	//broadcasted event when firing sequence ends
	UPROPERTY(BlueprintAssignable)
	FOnFinishFiring FinishedDelegate;


public:
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	int AmountOfProjectilesInAWave;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	float NumberOfWaves;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	float DurationBetweenWaves;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	float MaximumRadiusAroundPlayer;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	float AdditionalZHeight;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"), BlueprintReadWrite)
	UProjectileData* ProjData;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	TSubclassOf<AActor> IndicatorActorClass;

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ETraceTypeQuery> TraceType;


private:
	int WaveCounter;

	FTimerHandle TimerHandle;

	void StartNextTimer();
	
	void Fire();

	ABasicProjectile* CreateProjectile();
	
	bool bCurrentlyShooting;

	
	

};
