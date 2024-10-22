#pragma once

struct FWaveDivider;
DECLARE_DELEGATE_TwoParams(FOnWaveDividerTimeChange, uint32, float)

class SWaveDividerWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWaveDividerWidget)
		{
		}

		SLATE_ATTRIBUTE(TArray<FWaveDivider>, WaveDividerList)
		SLATE_ARGUMENT(uint32, SlotNumber)
		SLATE_EVENT(FOnWaveDividerTimeChange, OnWaveDividerTimeChange)
		// SLATE_ATTRIBUTE(uint32, SlotNumber)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	uint32 SlotNumber;

	void ConstructChildWidget();
	TSharedRef<SWidget> ConstructChildWidgetContent();

	TAttribute<TArray<FWaveDivider>> WaveDividerList;

	FOnWaveDividerTimeChange OnWaveDividerTimeChange;
};
