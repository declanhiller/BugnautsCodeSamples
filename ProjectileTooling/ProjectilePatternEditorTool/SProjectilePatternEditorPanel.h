#pragma once
#include "SProjectilePatternWaveInspector.h"


DECLARE_DELEGATE_TwoParams(FOnWaveDividerDistanceChanged, int32, float);
DECLARE_DELEGATE_OneParam(FOnWaveSelected, int32);
DECLARE_DELEGATE_OneParam(FOnWaveRemoved, int32)
DECLARE_DELEGATE_OneParam(FOnWaveAdded, int32)
DECLARE_DELEGATE_TwoParams(FOnWaveMoved, uint32, uint32)

class SProjectilePatternEditorPanel : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SProjectilePatternEditorPanel)
			: _WaveContainerList(TArray<FWaveContainer>())
			  , _WaveDividerList(TArray<FWaveDivider>())
		{
		}

		SLATE_ATTRIBUTE(TArray<FWaveContainer>, WaveContainerList)
		SLATE_ATTRIBUTE(TArray<FWaveDivider>, WaveDividerList)
		SLATE_EVENT(FOnWaveDividerDistanceChanged, OnWaveDividerDistanceChanged)
		SLATE_EVENT(FOnWaveSelected, OnWaveSelected)
		SLATE_EVENT(FOnWaveRemoved, OnWaveRemoved)
		SLATE_EVENT(FOnWaveAdded, OnWaveAdded)
		SLATE_EVENT(FOnWaveMoved, OnWaveMoved)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	TSharedRef<SWidget> CreateWaveDividerWidget(uint32 WaveDivider);

	TSharedRef<SWidget> CreateWaveContainerWidget(uint32 SlotNumber);

	TArray<FWaveContainer> GetWaveContainers() const;
	TArray<FWaveDivider> GetWaveDividers() const;

	void SetWaveDividerTimeValue(uint32 Index, float NewValue);

	void OpenWaveContainer(uint32 Index);
	void InsertNewWave(uint32 WaveIndex, int32 SlotIndex);
	void InsertNewWaveDivider(uint32 DividerIndex, int32 SlotIndex);
	void AddNewWave(uint32 WaveIndex);
	void AddNewDivider(uint32 DividerIndex);
	void RemoveSlot(uint32 SlotNumberToRemove);
	void RefreshSlotNumbers();
	uint32 NumSlots();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;


	TAttribute<TArray<FWaveContainer>> WaveContainerList;
	TAttribute<TArray<FWaveDivider>> WaveDividerList;

	FOnWaveDividerDistanceChanged OnWaveDividerDistanceChanged;
	FOnWaveSelected OnWaveSelected;
	FOnWaveRemoved OnWaveRemoved;
	FOnWaveAdded OnWaveAdded;
	FOnWaveMoved OnWaveMoved;

	uint32 CurrentContextMenuWaveNodeIndex;

	void FillSlots();


protected:

	TSharedPtr<SDragAndDropVerticalBox> DragAndDropVerticalBox;

	void CreateContextMenu(const FVector2D& mousePosition);

	void Delete();

	void AddAbove();

	void AddBelow();

	bool bRefreshOnNextTick;
	

	FReply HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, int32 SlotIndex,
	                          SVerticalBox::FSlot* Slot);
	TOptional<SDragAndDropVerticalBox::EItemDropZone> HandleCanAcceptDrop(
		const FDragDropEvent& DragDropEvent, SDragAndDropVerticalBox::EItemDropZone DropZone,
		SVerticalBox::FSlot* Slot);
	FReply HandleAcceptDrop(FDragDropEvent const& DragDropEvent, SDragAndDropVerticalBox::EItemDropZone DropZone,
	                        int32 SlotIndex, SVerticalBox::FSlot* Slot);
};

class FProjectilePatternEditorDragAndDropOp : public FDragAndDropVerticalBoxOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FProjectilePatternEditorDragAndDropOp, FDragAndDropVerticalBoxOp)

	// TSharedPtr<SWidget> WidgetToShow;

	// static TSharedRef<FProjectilePatternEditorDragAndDropOp> New(int32 InSlotIndexBeingDragged, SVerticalBox::FSlot* InSlotBeingDragged, TSharedPtr<SWidget> InWidgetToShow);

public:
	virtual ~FProjectilePatternEditorDragAndDropOp()
	{
	}

	// virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
};
