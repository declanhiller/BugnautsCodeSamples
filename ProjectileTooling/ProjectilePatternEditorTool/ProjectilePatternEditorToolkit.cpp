
#include "PatternData/ProjectilePatternEditorToolkit.h"

#include "ISinglePropertyView.h"
#include "PropertyCustomizationHelpers.h"
#include "SEditorViewport.h"

#include "Kismet/KismetMathLibrary.h"
#include "PatternData/Widgets/SProjectilePatternEditorPanel.h"
#include "PatternData/Widgets/SProjectilePatternViewport.h"
#include "PatternData/Widgets/SProjectilePatternWaveInspector.h"
#include "UObject/SavePackage.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "ProjectilePatternEditor"


void FProjectilePatternEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
	PatternData = Cast<UPatternData>(InObjects[0]);

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("PatternDataEditorLayout")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			                             ->Split
			                             (
				                             FTabManager::NewSplitter()
				                             ->SetSizeCoefficient(0.6f)
				                             ->SetOrientation(Orient_Horizontal)
				                             ->Split
				                             (
					                             FTabManager::NewStack()
					                             ->SetSizeCoefficient(0.8f)
					                             ->AddTab("PatternDataViewport", ETabState::OpenedTab)
				                             )
				                             ->Split
				                             (
					                             FTabManager::NewStack()
					                             ->SetSizeCoefficient(0.2f)
					                             ->AddTab("MainPatternDataEditor", ETabState::OpenedTab)
				                             )
			                             )
			                             ->Split
			                             (
				                             FTabManager::NewStack()
				                             ->SetSizeCoefficient(0.4f)
				                             ->AddTab("PatternDataEditorTab", ETabState::OpenedTab)
			                             )
		);
	FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, {}, "PatternDataEditor", Layout, true, true,
	                                     InObjects);
}

void FProjectilePatternEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& TabManagerIn)
{
	FAssetEditorToolkit::RegisterTabSpawners(TabManagerIn);

	WorkspaceMenuCategory = TabManagerIn->AddLocalWorkspaceMenuCategory(INVTEXT("Pattern Data Editor"));

	TabManagerIn->RegisterTabSpawner("PatternDataViewport", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
	            {
		            return SNew(SDockTab)[
			            SNew(SOverlay)
			            + SOverlay::Slot()[
				            SAssignNew(Viewport, SProjectilePatternViewport)

			            ]
			            + SOverlay::Slot()[
				            SNew(SBox)
				            .HAlign(HAlign_Right)
				            .VAlign(VAlign_Bottom)[
					            SNew(SButton)
					            .OnClicked(this, &FProjectilePatternEditorToolkit::PlayPattern)
					            .Content()
					            [
						            SNew(STextBlock)
						            .Text(LOCTEXT("Play", "Play"))
					            ]
				            ]
			            ]
		            ];
	            }))
	            .SetDisplayName(INVTEXT("Preview"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
	


	TabManagerIn->RegisterTabSpawner("PatternDataEditorTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
	            {
		            return SNew(SDockTab)
		            [

			            SAssignNew(Inspector, SProjectilePatternWaveInspector)
				            .WaveContainer(this, &FProjectilePatternEditorToolkit::GetCurrentWaveContainer)
				            .OnSetWaveDataInContainer(this, &FProjectilePatternEditorToolkit::SetWaveDataInWaveContainer)
				            .OnSetWaveDataV2InContainer(this, &FProjectilePatternEditorToolkit::SetWaveDataV2InWaveContainer)

		            ];
	            }))
	            .SetDisplayName(INVTEXT("Details"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());


	TabManagerIn->RegisterTabSpawner("MainPatternDataEditor", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
	            {
		            return SNew(SDockTab)[
			            SNew(SScrollBox)
			            .Orientation(Orient_Vertical)
			            .ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
			            .ScrollBarAlwaysVisible(true)

			            + SScrollBox::Slot()[
				            SAssignNew(EditorPanel, SProjectilePatternEditorPanel)
				            .WaveContainerList(this, &FProjectilePatternEditorToolkit::GetWaveContainers)
				            .WaveDividerList(this, &FProjectilePatternEditorToolkit::GetWaveDividers)
				            .OnWaveDividerDistanceChanged(
					            this, &FProjectilePatternEditorToolkit::SetWaveDividerDistanceTime)
				            .OnWaveSelected(this, &FProjectilePatternEditorToolkit::OpenUpWaveContainerWidget)
				            .OnWaveRemoved(this, &FProjectilePatternEditorToolkit::RemoveWaveContainer)
				            .OnWaveAdded(this, &FProjectilePatternEditorToolkit::AddWaveContainer)
				            .OnWaveMoved(this, &FProjectilePatternEditorToolkit::MoveWaveContainer)

			            ]
		            ];
	            }))
	            .SetDisplayName(INVTEXT("MainProjectilePatternEditor"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FProjectilePatternEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManagerIn)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManagerIn);
	TabManagerIn->UnregisterTabSpawner("PatternDataViewport");
	TabManagerIn->UnregisterTabSpawner("PatternDataEditorTab");
	TabManagerIn->UnregisterTabSpawner("MainPatternDataEditor");
}

TArray<FWaveContainer> FProjectilePatternEditorToolkit::GetWaveContainers() const
{
	return PatternData->WaveContainerList;
}

TArray<FWaveDivider> FProjectilePatternEditorToolkit::GetWaveDividers() const
{
	return PatternData->WaveDividerList;
}

FWaveContainer FProjectilePatternEditorToolkit::GetCurrentWaveContainer() const
{
	if (WaveIndex >= (uint32) PatternData->WaveContainerList.Num())
	{
		return FWaveContainer();
	}
	FWaveContainer& Container = PatternData->WaveContainerList[WaveIndex];
	return Container;
}

void FProjectilePatternEditorToolkit::SetWaveDividerDistanceTime(int32 Index, float NewValue)
{
	PatternData->Modify();
	FWaveDivider& Divider = PatternData->WaveDividerList[Index];
	Divider.DistanceTime = NewValue;
}

void FProjectilePatternEditorToolkit::OpenUpWaveContainerWidget(int32 IndexOfWave)
{
	UE_LOG(LogTemp, Warning, TEXT("Edit a wave hehehe"));
	WaveIndex = IndexOfWave;
	Inspector->RefreshWidget();
	// CustomizationClass->ShowNewWidget(IndexOfWave);
}

void FProjectilePatternEditorToolkit::RemoveWaveContainer(int32 IndexOfWave)
{
	PatternData->Modify();

	uint32 WaveSlotNumber = IndexOfWave;
	uint32 WaveDividerSlotNumber = PatternData->CalculateWaveDividerToRemove(WaveSlotNumber);
	if (PatternData->WaveContainerList.Num() - 1 == IndexOfWave)
	{
		EditorPanel->RemoveSlot(WaveSlotNumber * 2);
		EditorPanel->RemoveSlot(WaveDividerSlotNumber * 2 + 1);
	}
	else
	{
		EditorPanel->RemoveSlot(WaveSlotNumber * 2);
		EditorPanel->RemoveSlot(WaveDividerSlotNumber * 2);
	}

	PatternData->RemoveWave(IndexOfWave);

	// EditorPanel->RefreshSlotNumbers();
}

void FProjectilePatternEditorToolkit::AddWaveContainer(int32 IndexOfWave)
{
	PatternData->Modify();

	if (IndexOfWave == PatternData->WaveContainerList.Num())
	{
		EditorPanel->AddNewDivider(IndexOfWave);
		EditorPanel->AddNewWave(IndexOfWave);
	}
	else
	{
		EditorPanel->InsertNewWaveDivider(IndexOfWave, IndexOfWave * 2);
		EditorPanel->InsertNewWave(IndexOfWave, IndexOfWave * 2);
	}

	PatternData->AddWave(IndexOfWave);

	// EditorPanel->RefreshSlotNumbers();
}

void FProjectilePatternEditorToolkit::MoveWaveContainer(uint32 OriginIndex, uint32 DestinationIndex)
{
	PatternData->Modify();

	if (OriginIndex >= DestinationIndex)
	{
		uint32 IndexToGoTo = DestinationIndex / 2;

		if (DestinationIndex % 2 == 0)
		{
			PatternData->MoveWave(OriginIndex / 2, IndexToGoTo);
		}
		else
		{
			PatternData->MoveWave(OriginIndex / 2, IndexToGoTo + 1);
		}
	}
	else
	{
		DestinationIndex++;
		if (DestinationIndex - OriginIndex == 1 || DestinationIndex - OriginIndex == 0 || DestinationIndex - OriginIndex
			== 2)
		{
			//idek anymore bro, this shit is doing my head in
		}
		else if (DestinationIndex % 2 == 0)
		{
			PatternData->MoveWave(OriginIndex / 2, DestinationIndex / 2 - 1);
		}
		else
		{
			PatternData->MoveWave(OriginIndex / 2, DestinationIndex / 2);
		}
	}
}

FReply FProjectilePatternEditorToolkit::PlayPattern()
{
	Viewport->PlayPreviewPattern(PatternData);
	return FReply::Handled();
}

void FProjectilePatternEditorToolkit::SetWaveDataInWaveContainer(UWaveData* WaveData)
{
	PatternData->Modify();
	FWaveContainer& WaveContainer = PatternData->WaveContainerList[WaveIndex];
	WaveContainer.WaveData = WaveData;
	if (Inspector)
	{
		Inspector->RefreshWidget();
	}

	if (EditorPanel)
	{
		EditorPanel->Invalidate(EInvalidateWidgetReason::AttributeRegistration);
	}
}

void FProjectilePatternEditorToolkit::SetWaveDataV2InWaveContainer(UWaveDataV2* WaveData)
{
	PatternData->Modify();
	FWaveContainer& WaveContainer = PatternData->WaveContainerList[WaveIndex];
	WaveContainer.WaveDataV2 = WaveData;
	if (Inspector)
	{
		Inspector->RefreshWidget();
	}

	if (EditorPanel)
	{
		EditorPanel->Invalidate(EInvalidateWidgetReason::AttributeRegistration);
	}
}


#undef LOCTEXT_NAMESPACE
