#pragma once
#include "Bugnauts/ProjectileSystem/Data/PatternData.h"
#include "Bugnauts/ProjectileSystem/Data/WaveData.h"
#include "WaveData/WaveDataFactory.h"

DECLARE_DELEGATE_OneParam(FOnSetWaveDataInContainer, UWaveData*)
DECLARE_DELEGATE_OneParam(FOnSetWaveDataV2InContainer, UWaveDataV2*)

class SProjectilePatternWaveInspector : public SVerticalBox
{

	SLATE_BEGIN_ARGS(SProjectilePatternWaveInspector)
	{
	}

	SLATE_ATTRIBUTE(FWaveContainer, WaveContainer);
	SLATE_EVENT(FOnSetWaveDataInContainer, OnSetWaveDataInContainer);
	SLATE_EVENT(FOnSetWaveDataV2InContainer, OnSetWaveDataV2InContainer);

SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs);

	virtual ~SProjectilePatternWaveInspector() override;

	TAttribute<FWaveContainer> WaveContainer; //You can pass in a method that returns the wave container being edited
	//and have only the toolkit know about which divider is selected maybe?

	TSharedPtr<IDetailsView> DetailView;

	void RefreshWidget();

	FString GetSelectedWaveDataPath() const;
	void SetSelectedWaveData(const FAssetData& InAssetData);

	FString GetSelectedWaveDataV2Path() const;
	void SetSelectedWaveDataV2Path(const FAssetData& InAssetData);

	FOnSetWaveDataInContainer OnSetWaveDataInContainer;
	FOnSetWaveDataV2InContainer OnSetWaveDataV2InContainer;

	UWaveDataFactory* WaveDataFactory;

	class UWaveDataV2Factory* WaveDataV2Factory;

};

