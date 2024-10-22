#pragma once

struct FWaveContainer;
DECLARE_DELEGATE_OneParam(FOnClick, uint32)

class SWaveContainerWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWaveContainerWidget)
	{
	}

	SLATE_ATTRIBUTE(TArray<FWaveContainer>, WaveContainerList)
	SLATE_ARGUMENT(uint32, SlotNumber)
	SLATE_EVENT(FOnClick, OnClick)
	// SLATE_ATTRIBUTE(uint32, SlotNumber)
	
	SLATE_END_ARGS()
	
	
public:

	void Construct(const FArguments& InArgs);
	
	uint32 SlotNumber;

	void ConstructChildWidget();
	TSharedRef<SWidget> ConstructChildWidgetContent();

	TAttribute<TArray<FWaveContainer>> WaveContainerList;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

protected:
	FOnClick OnClick;
	
};
