#include "PatternData/Widgets/SProjectilePatternViewport.h"

#include "CameraController.h"
#include "MockObjectPoolSubsystem.h"
#include "UnrealEdGlobals.h"
#include "Bugnauts/ProjectileSystem/ProjectileSpawner.h"
#include "Bugnauts/ProjectileSystem/ProjectileSpawnerController.h"
#include "Components/DirectionalLightComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "GameFramework/WorldSettings.h"
#include "OodleDataCompression/Sdks/2.9.7/include/oodle2base.h"
#include "ThumbnailRendering/ThumbnailManager.h"


class FProjectilePatternViewportClient : public FEditorViewportClient
{
public:
	FProjectilePatternViewportClient(FPreviewScene* InPreviewScene,
	                                 const TSharedRef<SProjectilePatternViewport>& InViewport);

private:
	FPreviewScene* PreviewScene;

public:
	virtual void Tick(float DeltaSeconds) override;
};

class FProjectilePatternEditorPreviewScene : public FPreviewScene
{
public:
	FProjectilePatternEditorPreviewScene(ConstructionValues CVS);
	~FProjectilePatternEditorPreviewScene();

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

public:
	UProjectileSpawnerController* SpawnerController;

protected:
	AActor* PreviewActor;
	UProjectileSpawner* Spawner;
	USceneComponent* SceneComponent;
	UMockObjectPoolSubsystem* ObjectPool;
	TArray<AActor*> CurrentProjectiles;

	void AddProjectile(ABasicProjectile* Projectile)
	{
		CurrentProjectiles.Add(Projectile);
	}

	void DeleteProjectile(ABasicProjectile* Projectile)
	{
		CurrentProjectiles.Remove(Projectile);
	}
};


FProjectilePatternViewportClient::FProjectilePatternViewportClient(FPreviewScene* InPreviewScene,
                                                                   const TSharedRef<SProjectilePatternViewport>&
                                                                   InViewport) :
	FEditorViewportClient(nullptr, InPreviewScene, StaticCastSharedRef<SEditorViewport>(InViewport))
{
	PreviewScene = InPreviewScene;
	if (!this->IsSetShowGridChecked())
	{
		this->SetShowGrid();
	}

	FRotator CameraRotation = FRotator(-45, 45, 0);

	
	SetViewLocation(-CameraRotation.Vector() * 1500);
	SetViewRotation(CameraRotation);

	bUsingOrbitCamera = true;
	// this->bIsRealtime = true;
	this->SetRealtime(true);
}

void FProjectilePatternViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	// Tick the preview scene world.
	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

FProjectilePatternEditorPreviewScene::FProjectilePatternEditorPreviewScene(ConstructionValues CVS)
{
	
	// world setting
	GetWorld()->GetWorldSettings()->NotifyBeginPlay();
	GetWorld()->GetWorldSettings()->NotifyMatchStarted();
	GetWorld()->GetWorldSettings()->SetActorHiddenInGame(false);
	GetWorld()->bBegunPlay = true;
	// set light options 
	DirectionalLight->SetRelativeLocation(FVector(-1024.f, 1024.f, 2048.f));
	DirectionalLight->SetRelativeScale3D(FVector(15.f));

	SetLightBrightness(4.f);
	DirectionalLight->InvalidateLightingCache();
	DirectionalLight->RecreateRenderState_Concurrent();

	// creae a sky sphere
	UStaticMeshComponent* SkyComp = NewObject<UStaticMeshComponent>();
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/MapTemplates/Sky/SM_SkySphere.SM_SkySphere"),
	                                                  NULL, LOAD_None, NULL);
	SkyComp->SetStaticMesh(StaticMesh);
	// UMaterial* SkyMaterial = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaSky.PersonaSky"), NULL, LOAD_None, NULL);
	// SkyComp->SetMaterial(0, SkyMaterial);
	const float SkySphereScale = 1000.f;
	const FTransform SkyTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(SkySphereScale));
	AddComponent(SkyComp, SkyTransform);

	// this->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);


	// now add floor
	// UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube"), NULL, LOAD_None, NULL);
	// UStaticMeshComponent* FloorComp = NewObject<UStaticMeshComponent>();
	// FloorComp->SetStaticMesh(FloorMesh);
	// AddComponent(FloorComp, FTransform::Identity);
	// FloorComp->SetRelativeScale3D(FVector(3.f, 3.f, 1.f));
	// UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaFloorMat.PersonaFloorMat"), NULL, LOAD_None, NULL);
	// FloorComp->SetMaterial(0, Material);
	// FloorComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// FloorComp->SetCollisionObjectType(ECC_WorldStatic);

	//actual actor setup
	UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>();
	UStaticMesh* MeshAsset = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube"), NULL,
	                                                 LOAD_None, NULL);
	StaticMeshComponent->SetStaticMesh(MeshAsset);

	AddComponent(StaticMeshComponent, FTransform::Identity);

	// UProjectileSpawner* Spawner = NewObject<UProjectileSpawner>();
	// UProjectileSpawnerController* SpawnerController = NewObject<UProjectileSpawnerController>();
	// SpawnerController->ProjectileSpawner = Spawner;
	// AddComponent(Spawner, FTransform::Identity);
	// AddComponent(SpawnerController, FTransform::Identity);


	PreviewActor = GetWorld()->SpawnActor(AActor::StaticClass());
	Spawner = Cast<UProjectileSpawner>(
		PreviewActor->AddComponentByClass(UProjectileSpawner::StaticClass(), false, FTransform::Identity, false));
	SpawnerController = Cast<UProjectileSpawnerController>(
		PreviewActor->AddComponentByClass(UProjectileSpawnerController::StaticClass(), false, FTransform::Identity,
		                                  false));
	SceneComponent = Cast<USceneComponent>(
		PreviewActor->AddComponentByClass(USceneComponent::StaticClass(), false, FTransform::Identity, false));
	Spawner->Firepoint = SceneComponent;

	UMockObjectPoolSubsystem* MockPool = GEditor->GetEditorSubsystem<UMockObjectPoolSubsystem>();

	MockPool->OnCreatedProjectile.BindLambda([this](AActor* Projectile)
	{
		CurrentProjectiles.Add(Projectile);
	});
	MockPool->OnDeletedProjectile.BindLambda([this](AActor* Projectile)
	{
		CurrentProjectiles.Remove(Projectile);
	});

	ObjectPool = MockPool;
	ObjectPool->World = GetWorld();
	Spawner->SetupObjectPool(ObjectPool);
	SpawnerController->ProjectileSpawner = Spawner;
}

FProjectilePatternEditorPreviewScene::~FProjectilePatternEditorPreviewScene()
{
	ObjectPool->OnCreatedProjectile.Unbind();
	ObjectPool->OnDeletedProjectile.Unbind();
}

void FProjectilePatternEditorPreviewScene::AddReferencedObjects(FReferenceCollector& Collector)
{
	FPreviewScene::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(PreviewActor);
	Collector.AddReferencedObject(ObjectPool);
	Collector.AddReferencedObjects(CurrentProjectiles);
}


TSharedRef<FEditorViewportClient> SProjectilePatternViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(
		new FProjectilePatternViewportClient(PreviewScene.Get(),
		                                     SharedThis(this)));

	// EditorViewportClient = MakeShareable(new FProjectilePatternViewportClient(nullptr, SharedThis(this)));

	return EditorViewportClient.ToSharedRef();
}

void SProjectilePatternViewport::Construct(const FArguments& InArgs)
{
	PreviewScene = MakeShareable(new FProjectilePatternEditorPreviewScene(FPreviewScene::ConstructionValues()));

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SProjectilePatternViewport::PlayPreviewPattern(UPatternData* PatternData)
{
	StaticCastSharedPtr<FProjectilePatternEditorPreviewScene>(PreviewScene)->SpawnerController->Fire(PatternData);
}
