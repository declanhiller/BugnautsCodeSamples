#include "PatternData/Widgets/SProjectilePatternEditorPanel.h"

#include "SWaveContainerWidget.h"
#include "SWaveDividerWidget.h"

#define LOCTEXT_NAMESPACE "ProjectilePatternEditorPanel"


struct FWaveDivider;
struct FWaveContainer;

void SProjectilePatternEditorPanel::Construct(const FArguments& InArgs)
{
	WaveContainerList = InArgs._WaveContainerList;
	WaveDividerList = InArgs._WaveDividerList;
	OnWaveDividerDistanceChanged = InArgs._OnWaveDividerDistanceChanged;
	OnWaveSelected = InArgs._OnWaveSelected;
	OnWaveRemoved = InArgs._OnWaveRemoved;
	OnWaveAdded = InArgs._OnWaveAdded;
	OnWaveMoved = InArgs._OnWaveMoved;


	TSharedRef<SDragAndDropVerticalBox> Box = SNew(SDragAndDropVerticalBox)
	.OnCanAcceptDrop(this, &SProjectilePatternEditorPanel::HandleCanAcceptDrop)
	.OnAcceptDrop(this, &SProjectilePatternEditorPanel::HandleAcceptDrop)
	.OnDragDetected(this, &SProjectilePatternEditorPanel::HandleDragDetected);

	Box->SetDropIndicator_Above(*FAppStyle::GetBrush("LandscapeEditor.TargetList.DropZone.Above"));
	Box->SetDropIndicator_Below(*FAppStyle::GetBrush("LandscapeEditor.TargetList.DropZone.Below"));

	ChildSlot[
		Box
	];

	DragAndDropVerticalBox = Box;

	FillSlots();
}


FReply SProjectilePatternEditorPanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	DragAndDropVerticalBox->ArrangeChildren(MyGeometry, ArrangedChildren);

	int32 NodeUnderPositionIndex = SWidget::FindChildUnderMouse(ArrangedChildren, MouseEvent);


	if (NodeUnderPositionIndex / 2 < WaveContainerList.Get().Num() && NodeUnderPositionIndex % 2 == 0)
	{
		CurrentContextMenuWaveNodeIndex = NodeUnderPositionIndex / 2;
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			CreateContextMenu(MouseEvent.GetScreenSpacePosition());
			UE_LOG(LogTemp, Warning, TEXT("Right Clicked"));
			return FReply::Unhandled();
		}
		OnWaveSelected.ExecuteIfBound(NodeUnderPositionIndex / 2);
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> SProjectilePatternEditorPanel::CreateWaveDividerWidget(uint32 SlotNumber)
{
	return SNew(SWaveDividerWidget)
	.SlotNumber(SlotNumber)
	.WaveDividerList(this, &SProjectilePatternEditorPanel::GetWaveDividers)
	.OnWaveDividerTimeChange(this, &SProjectilePatternEditorPanel::SetWaveDividerTimeValue);
}

TSharedRef<SWidget> SProjectilePatternEditorPanel::CreateWaveContainerWidget(uint32 SlotNumber)
{
	return SNew(SBox)[
		SNew(SWaveContainerWidget)
		.SlotNumber(SlotNumber)
		.WaveContainerList(this, &SProjectilePatternEditorPanel::GetWaveContainers)
		.OnClick(this, &SProjectilePatternEditorPanel::OpenWaveContainer)
	].MinDesiredHeight(50);
}

TArray<FWaveContainer> SProjectilePatternEditorPanel::GetWaveContainers() const
{
	return WaveContainerList.Get();
}

TArray<FWaveDivider> SProjectilePatternEditorPanel::GetWaveDividers() const
{
	return WaveDividerList.Get();
}

void SProjectilePatternEditorPanel::SetWaveDividerTimeValue(uint32 Index, float NewValue)
{
	OnWaveDividerDistanceChanged.ExecuteIfBound(Index, NewValue);
}

void SProjectilePatternEditorPanel::OpenWaveContainer(uint32 Index)
{
	OnWaveSelected.ExecuteIfBound(Index);
}

void SProjectilePatternEditorPanel::InsertNewWave(uint32 WaveIndex, int32 SlotIndex)
{
	DragAndDropVerticalBox->InsertSlot(SlotIndex)[
		CreateWaveContainerWidget(WaveIndex)
	];
}

void SProjectilePatternEditorPanel::InsertNewWaveDivider(uint32 DividerIndex, int32 SlotIndex)
{
	DragAndDropVerticalBox->InsertSlot(SlotIndex)[
		CreateWaveDividerWidget(DividerIndex)
	].MaxHeight(30);
}

void SProjectilePatternEditorPanel::AddNewWave(uint32 WaveIndex)
{
	DragAndDropVerticalBox->AddSlot()[
		CreateWaveContainerWidget(WaveIndex)
	];
}

void SProjectilePatternEditorPanel::AddNewDivider(uint32 DividerIndex)
{
	DragAndDropVerticalBox->AddSlot()[
		CreateWaveDividerWidget(DividerIndex)
	].MaxHeight(30);
}

void SProjectilePatternEditorPanel::RefreshSlotNumbers()
{
	for (int i = 0; i < DragAndDropVerticalBox->NumSlots(); i++)
	{
		TSharedRef<SWidget> Widget = DragAndDropVerticalBox->GetSlot(i).GetWidget();
		if (i % 2 == 0)
		{
			TSharedRef<SWaveContainerWidget> WaveContainer = StaticCastSharedRef<SWaveContainerWidget>(Widget);
			WaveContainer->SlotNumber = i / 2;
		}
		else
		{
			TSharedRef<SWaveDividerWidget> WaveDivider = StaticCastSharedRef<SWaveDividerWidget>(Widget);
			WaveDivider->SlotNumber = i / 2;
		}
	}
}

uint32 SProjectilePatternEditorPanel::NumSlots()
{
	return DragAndDropVerticalBox->NumSlots();
}

void SProjectilePatternEditorPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                         const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (bRefreshOnNextTick)
	{
		FillSlots();
		bRefreshOnNextTick = false;
	}
}

void SProjectilePatternEditorPanel::RemoveSlot(uint32 SlotNumberToRemove)
{
	DragAndDropVerticalBox->RemoveSlot(DragAndDropVerticalBox->GetSlot(SlotNumberToRemove).GetWidget());
}

void SProjectilePatternEditorPanel::FillSlots()
{
	DragAndDropVerticalBox->ClearChildren();

	TArray<FWaveContainer> Waves = WaveContainerList.Get();
	TArray<FWaveDivider> WaveDividers = WaveDividerList.Get();

	for (int i = 0; i < Waves.Num(); i++)
	{
		DragAndDropVerticalBox->AddSlot()[
			CreateWaveContainerWidget(i)
		];
		if (i == Waves.Num() - 1) continue;
		DragAndDropVerticalBox->AddSlot()
		                      .MaxHeight(30)[
			CreateWaveDividerWidget(i)

		];
	}
}

void SProjectilePatternEditorPanel::CreateContextMenu(const FVector2D& mousePosition)
{
	constexpr bool bCloseAfterSelection = true;

	FMenuBuilder MenuBuilder(bCloseAfterSelection, nullptr);

	MenuBuilder.BeginSection("MainFunctions");
	{
		if (WaveContainerList.Get().Num() > 1)
		{
			TSharedRef<SWidget> DeleteWidget = SNew(STextBlock)
				.Text(LOCTEXT("Delete", "Delete"));
			MenuBuilder.AddMenuEntry(FUIAction(FExecuteAction::CreateSP(this, &SProjectilePatternEditorPanel::Delete)),
			                         DeleteWidget);
		}


		TSharedRef<SWidget> AddAboveWidget = SNew(STextBlock)
			.Text(LOCTEXT("AddAbove", "Add Above"));
		MenuBuilder.AddMenuEntry(FUIAction(FExecuteAction::CreateSP(this, &SProjectilePatternEditorPanel::AddAbove)),
		                         AddAboveWidget);

		TSharedRef<SWidget> AddBelowWidget = SNew(STextBlock)
			.Text(LOCTEXT("AddBelow", "Add Below"));
		MenuBuilder.AddMenuEntry(FUIAction(FExecuteAction::CreateSP(this, &SProjectilePatternEditorPanel::AddBelow)),
		                         AddBelowWidget);
	}
	MenuBuilder.EndSection();

	FSlateApplication::Get().PushMenu(
		SharedThis(this),
		FWidgetPath(),
		MenuBuilder.MakeWidget(),
		mousePosition,
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

void SProjectilePatternEditorPanel::Delete()
{
	OnWaveRemoved.ExecuteIfBound(CurrentContextMenuWaveNodeIndex);
	bRefreshOnNextTick = true;
}

void SProjectilePatternEditorPanel::AddAbove()
{
	OnWaveAdded.ExecuteIfBound(CurrentContextMenuWaveNodeIndex);
	bRefreshOnNextTick = true;
}

void SProjectilePatternEditorPanel::AddBelow()
{
	OnWaveAdded.ExecuteIfBound(CurrentContextMenuWaveNodeIndex + 1);
	bRefreshOnNextTick = true;
}


FReply SProjectilePatternEditorPanel::HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent,
                                                         int32 SlotIndex, SVerticalBox::FSlot* Slot)
{
	TSharedRef<FProjectilePatternEditorDragAndDropOp> Operation = MakeShareable(
		new FProjectilePatternEditorDragAndDropOp);
	if (SlotIndex % 2 == 0)
	{
		Operation->SlotIndexBeingDragged = SlotIndex;
		Operation->SlotBeingDragged = Slot;
		return FReply::Handled().BeginDragDrop(Operation);
	}

	return FReply::Unhandled();
}

TOptional<SDragAndDropVerticalBox::EItemDropZone> SProjectilePatternEditorPanel::HandleCanAcceptDrop(
	const FDragDropEvent& DragDropEvent, SDragAndDropVerticalBox::EItemDropZone DropZone, SVerticalBox::FSlot* Slot)
{
	TSharedPtr<FProjectilePatternEditorDragAndDropOp> DragDropOperation = DragDropEvent.GetOperationAs<
		FProjectilePatternEditorDragAndDropOp>();
	if (DragDropOperation.IsValid())
	{
		return DropZone;
	}
	return TOptional<SDragAndDropVerticalBox::EItemDropZone>();
}

FReply SProjectilePatternEditorPanel::HandleAcceptDrop(FDragDropEvent const& DragDropEvent,
                                                       SDragAndDropVerticalBox::EItemDropZone DropZone,
                                                       int32 SlotIndex, SVerticalBox::FSlot* Slot)
{
	TSharedPtr<FProjectilePatternEditorDragAndDropOp> DragDropOperation = DragDropEvent.GetOperationAs<
		FProjectilePatternEditorDragAndDropOp>();

	if (DragDropOperation.IsValid())
	{
		uint32 OriginIndex = DragDropOperation->SlotIndexBeingDragged;
		uint32 DestinationIndex = SlotIndex;

		OnWaveMoved.ExecuteIfBound(OriginIndex, DestinationIndex);

		bRefreshOnNextTick = true;

		return FReply::Handled();
	}

	return FReply::Unhandled();
}


#undef LOCTEXT_NAMESPACE
