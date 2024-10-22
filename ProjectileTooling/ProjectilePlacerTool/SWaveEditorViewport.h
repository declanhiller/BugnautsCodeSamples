#pragma once
#include "SEditorViewport.h"
#include "WaveDataV2EditorToolkit.h"
#include "Bugnauts/ProjectileSystem/Data/WaveShapes/WaveShape.h"

class SWaveEditorViewport : public SEditorViewport
{
	SLATE_BEGIN_ARGS(SWaveEditorViewport)
	{
	}

	SLATE_ARGUMENT(TWeakPtr<FWaveDataV2EditorToolkit>, WaveDataEditor)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	
	void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);

	~SWaveEditorViewport();

	TSharedPtr<class FWaveDataEditorViewportClient> GetViewportClient() {return ViewportClient;}

	void RegenerateWaveShapes();
	void AddWaveShape(UWaveShape* WaveShape);
	void DeleteWaveShape(UWaveShape* WaveShape);
	void UpdateWaveShape(UWaveShape* WaveShape);

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

private:
	TSharedPtr<class FWaveDataEditorViewportClient> ViewportClient;

	TWeakPtr<FWaveDataV2EditorToolkit> WaveDataEditor;

	FPreviewScene PreviewScene;

	FDelegateHandle PreviewFeatureLevelChangedHandle;
	
};
