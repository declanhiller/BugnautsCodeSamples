#pragma once
#include "Bugnauts/ProjectileSystem/Data/PatternData.h"

class SProjectilePatternEditorPanel;
class SProjectilePatternWaveInspector;
class SProjectilePatternViewport;

class FProjectilePatternEditorToolkit : public FAssetEditorToolkit
{
public:
	void InitEditor(const TArray<UObject*>& InObjects);

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	
	virtual FName GetToolkitFName() const override { return "PatternDataEditor"; }
	virtual FText GetBaseToolkitName() const override { return INVTEXT("Pattern Data Editor"); }
	virtual FString GetWorldCentricTabPrefix() const override { return "Pattern Data "; }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return {}; }

	TArray<FWaveContainer> GetWaveContainers() const;
	TArray<FWaveDivider> GetWaveDividers() const;
	FWaveContainer GetCurrentWaveContainer() const;
	void SetWaveDividerDistanceTime(int32 Index, float NewValue);

	void OpenUpWaveContainerWidget(int32 IndexOfWave);
	void RemoveWaveContainer(int32 IndexOfWave);
	void AddWaveContainer(int32 IndexOfWave);
	void MoveWaveContainer(uint32 OriginIndex, uint32 DestinationIndex);

	FReply PlayPattern();

	void SetWaveDataInWaveContainer(UWaveData* WaveData);
	void SetWaveDataV2InWaveContainer(UWaveDataV2* WaveData);

private:
	UPatternData* PatternData;

	SVerticalBox* VerticalBox;

	TSharedPtr<SProjectilePatternWaveInspector> Inspector;

	TSharedPtr<SProjectilePatternEditorPanel> EditorPanel;

	TSharedPtr<SProjectilePatternViewport> Viewport;

	uint32 WaveIndex;
	
};
