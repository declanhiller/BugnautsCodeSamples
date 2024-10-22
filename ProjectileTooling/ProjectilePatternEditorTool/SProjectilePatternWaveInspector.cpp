#include "PatternData/Widgets/SProjectilePatternWaveInspector.h"

#include "PropertyCustomizationHelpers.h"
#include "WaveDataV2Factory.h"
#include "Bugnauts/ProjectileSystem/Data/PatternData.h"
#include "Factories/DataAssetFactory.h"
#include "WaveData/WaveDataFactory.h"

void SProjectilePatternWaveInspector::Construct(const FArguments& InArgs)
{
	WaveContainer = InArgs._WaveContainer;
	OnSetWaveDataInContainer = InArgs._OnSetWaveDataInContainer;
	OnSetWaveDataV2InContainer = InArgs._OnSetWaveDataV2InContainer;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedRef<IDetailsView> CreatedDetailView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailView = CreatedDetailView;

	DetailView->GetWidgetClass();

	WaveDataFactory = NewObject<UWaveDataFactory>();
	WaveDataFactory->AddToRoot();

	TArray<UFactory*> Temp = TArray<UFactory*>();
	Temp.Add(WaveDataFactory);
	TOptional<TArray<UFactory*>> WaveDataFactories = TOptional(Temp);

	WaveDataV2Factory = NewObject<UWaveDataV2Factory>();
	WaveDataV2Factory->AddToRoot();

	TArray<UFactory*> TempV2 = TArray<UFactory*>();
	TempV2.Add(WaveDataFactory);
	TOptional<TArray<UFactory*>> WaveDataV2Factories = TOptional(TempV2);

	AddSlot()[
		SNew(SBox)[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UWaveData::StaticClass())
			.AllowCreate(true)
			.AllowClear(true)
			.ObjectPath(this, &SProjectilePatternWaveInspector::GetSelectedWaveDataPath)
			.OnObjectChanged(this, &SProjectilePatternWaveInspector::SetSelectedWaveData)
			.NewAssetFactories(WaveDataFactories)
		]
		
	].MaxHeight(30);

	AddSlot()[
		SNew(SBox)[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UWaveDataV2::StaticClass())
			.AllowCreate(true)
			.AllowClear(true)
			.ObjectPath(this, &SProjectilePatternWaveInspector::GetSelectedWaveDataV2Path)
			.OnObjectChanged(this, &SProjectilePatternWaveInspector::SetSelectedWaveDataV2Path)
			.NewAssetFactories(TempV2)
			]
	].MaxHeight(30);

	AddSlot()
	[
		CreatedDetailView
	];
}

SProjectilePatternWaveInspector::~SProjectilePatternWaveInspector()
{
	if(WaveDataFactory)
	{
		WaveDataFactory->RemoveFromRoot();
		WaveDataFactory = nullptr;
	}
}

void SProjectilePatternWaveInspector::RefreshWidget()
{
	if (WaveContainer.IsBound())
	{
		DetailView->SetObject(WaveContainer.Get().WaveData);
	}
}

FString SProjectilePatternWaveInspector::GetSelectedWaveDataPath() const
{
	if (!WaveContainer.Get().WaveData)
	{
		FString EmptyPath = FString("");
		return EmptyPath;
	}
	return WaveContainer.Get().WaveData->GetPathName();
}

void SProjectilePatternWaveInspector::SetSelectedWaveData(const FAssetData& InAssetData)
{
	if (DetailView.IsValid())
	{
		UWaveData* SelectedWaveData = Cast<UWaveData>(InAssetData.GetAsset());
		if (SelectedWaveData)
		{
			OnSetWaveDataInContainer.ExecuteIfBound(SelectedWaveData);
		}
	}
}

FString SProjectilePatternWaveInspector::GetSelectedWaveDataV2Path() const
{
	if (!WaveContainer.Get().WaveDataV2)
	{
		FString EmptyPath = FString("");
		return EmptyPath;
	}
	return WaveContainer.Get().WaveDataV2->GetPathName();
}

void SProjectilePatternWaveInspector::SetSelectedWaveDataV2Path(const FAssetData& InAssetData)
{
	if (DetailView.IsValid())
	{
		UWaveDataV2* SelectedWaveData = Cast<UWaveDataV2>(InAssetData.GetAsset());
		if (SelectedWaveData)
		{
			OnSetWaveDataV2InContainer.ExecuteIfBound(SelectedWaveData);
		}
	}
}
