#include "WaveDataEditorViewportClient.h"

#include "MockObjectPoolSubsystem.h"
#include "UnrealEdGlobals.h"
#include "UnrealWidget.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "ThumbnailRendering/ThumbnailManager.h"

class UEditorPerProjectUserSettings;

FWaveDataEditorViewportClient::FWaveDataEditorViewportClient(TWeakPtr<FWaveDataV2EditorToolkit> InWaveDataEditor,
                                                             FPreviewScene* InPreviewScene, const TSharedRef<SWaveEditorViewport>& InViewport)
		: FEditorViewportClient(nullptr, InPreviewScene, StaticCastSharedRef<SEditorViewport>(InViewport))
{
	WaveDataEditor = InWaveDataEditor;

	EngineShowFlags.DisableAdvancedFeatures();

	check(Widget);
	Widget->SetSnapEnabled(true);

	// Selectively set particular show flags that we need
	EngineShowFlags.SetSelectionOutline(GetDefault<ULevelEditorViewportSettings>()->bUseSelectionOutline);

	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowGrid;

	GlobalArrowComponent = NewObject<UArrowComponent>(GetTransientPackage(), TEXT("Global Arrow"));
	GlobalArrowComponent->bSelectable = false;
	GlobalArrowComponent->ArrowColor = FColor::Red;
	// GlobalArrowComponent->ArrowSize
	PreviewScene->AddComponent(GlobalArrowComponent, FTransform::Identity);
	
	// now add floor
	EditorFloorComp = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("EditorFloorComp"));

	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/PhAT_FloorBox.PhAT_FloorBox"),
	                                                 NULL, LOAD_None, NULL);
	if (ensure(FloorMesh))
	{
		EditorFloorComp->SetStaticMesh(FloorMesh);
	}

	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaFloorMat.PersonaFloorMat"),
	                                            NULL, LOAD_None, NULL);
	if (ensure(Material))
	{
		EditorFloorComp->SetMaterial(0, Material);
	}

	EditorFloorComp->SetRelativeScale3D(FVector(3.f, 3.f, 1.f));
	EditorFloorComp->SetVisibility(GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowFloor);
	EditorFloorComp->SetCollisionEnabled(GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowFloor
		                                     ? ECollisionEnabled::QueryAndPhysics
		                                     : ECollisionEnabled::NoCollision);
	PreviewScene->AddComponent(EditorFloorComp, FTransform::Identity);

	// AnchorSphereComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("AnchorSphereComponent"));
	// UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube"),
	//                                                   NULL, LOAD_None, NULL);
	// if(ensure(SphereMesh))
	// {
	// 	AnchorSphereComponent->SetStaticMesh(SphereMesh);
	// }
	//
	//
	// PreviewScene->AddComponent(AnchorSphereComponent, FTransform::Identity);


	// Turn off so that actors added to the world do not have a lifespan (so they will not auto-destroy themselves).
	PreviewScene->GetWorld()->bBegunPlay = false;

	PreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);


	//Toggle camera position and stuff
	// ToggleOrbitCamera(false);
	//
	//
	SetViewLocation(FVector(0.0f, 0.0f, 500.0f));
	SetViewRotation(FRotator(-90, 0, 0));
	//
	// ViewportType = ELevelViewportType::LVT_OrthoXY;
	SetViewMode(VMI_Lit);
	
	
	
}

void FWaveDataEditorViewportClient::InvalidatePreview(bool bResetCamera)
{
	Invalidate();
}

void FWaveDataEditorViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEditorViewportClient::AddReferencedObjects(Collector);
	Collector.AddReferencedObjects(CurrentProjectiles);
}


AActor* FWaveDataEditorViewportClient::CreateProjectile(FWaveProjectileContainer Container)
{
	AActor* Projectile = GetWorld()->SpawnActor<AActor>();
	UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(
		Projectile->AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform::Identity, false));
	UStaticMesh* MeshAsset = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/EditorSphere.EditorSphere"), NULL,
													 LOAD_None, NULL);

	UArrowComponent* ArrowComponent = Cast<UArrowComponent>(Projectile->AddComponentByClass(UArrowComponent::StaticClass(), false, FTransform::Identity, false));
	
	MeshComponent->SetStaticMesh(MeshAsset);

	Projectile->SetActorLocation(Container.LocalPositionToSpawn);
	ArrowComponent->SetWorldRotation(Container.LocalDirectionToFire.Rotation());
	ArrowComponent->bSelectable = false;
	// ArrowComponent->SetArrowLength(300);
	ArrowComponent->SetArrowSize(5);
	ArrowComponent->SetArrowColor(FColor::Blue);
	Projectile->SetActorScale3D(FVector(0.03f, 0.03f, 0.03f));
	
	return Projectile;
}

void FWaveDataEditorViewportClient::RegenerateWaveShapePreviewContainers()
{
	UWaveDataV2* WaveData = WaveDataEditor.Pin()->GetWaveData();
	WaveData->Modify();
	WaveData->ProjectileContainers.Empty();
	for (AActor* Element : CurrentProjectiles)
	{
		Element->Destroy();
	}
	CurrentProjectiles.Empty();
	PreviewContainers.Empty();
	
	TArray<FWaveProjectileContainer> AllProjectiles;
	for (UWaveShape* WaveShape : WaveData->Shapes)
	{
		FWaveShapePreviewContainer Container;
		TArray<FWaveProjectileContainer> SpecificProjectilesContainers;
		SpecificProjectilesContainers = WaveShape->CalculateProjectilesFromShape();
		Container.WaveShape = WaveShape;
		for (FWaveProjectileContainer SpecificProjectileContainer : SpecificProjectilesContainers)
		{
			AActor* Projectile = CreateProjectile(SpecificProjectileContainer);
			CurrentProjectiles.Add(Projectile);
			Container.PreviewProjectiles.Add(Projectile);
		}
		AllProjectiles.Append(SpecificProjectilesContainers);
	}
	WaveData->ProjectileContainers.Append(AllProjectiles);
	InvalidatePreview();
}

void FWaveDataEditorViewportClient::AddWaveShape(UWaveShape* WaveShape)
{
	// FWaveShapePreviewContainer PreviewContainer
}

void FWaveDataEditorViewportClient::DeleteWaveShape(UWaveShape* WaveShape)
{
}

void FWaveDataEditorViewportClient::UpdateWaveShape(UWaveShape* WaveShape)
{
}

