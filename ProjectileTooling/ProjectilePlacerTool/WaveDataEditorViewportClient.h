#pragma once
#include "SWaveEditorViewport.h"
#include "Components/ArrowComponent.h"


struct FWaveShapePreviewContainer
{

	UWaveShape* WaveShape;

	TArray<AActor*> PreviewProjectiles;
	
};

class FWaveDataEditorViewportClient : public FEditorViewportClient , public TSharedFromThis<FEditorViewportClient>
{
public:
	FWaveDataEditorViewportClient(TWeakPtr<FWaveDataV2EditorToolkit> InWaveDataEditor, FPreviewScene* InPreviewScene, const TSharedRef<SWaveEditorViewport>& InViewport);


	void InvalidatePreview(bool bResetCamera = true);


	UStaticMeshComponent* EditorFloorComp;
	UStaticMeshComponent* AnchorSphereComponent;
	UArrowComponent* GlobalArrowComponent;

	TArray<AActor*> CurrentProjectiles;

	TArray<FWaveShapePreviewContainer> PreviewContainers;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	
	AActor* CreateProjectile(FWaveProjectileContainer Container);

	void RegenerateWaveShapePreviewContainers();
	void AddWaveShape(UWaveShape* WaveShape);
	void DeleteWaveShape(UWaveShape* WaveShape);
	void UpdateWaveShape(UWaveShape* WaveShape);


private:
	TWeakPtr<class FWaveDataV2EditorToolkit> WaveDataEditor;

	
};
