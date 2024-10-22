#include "SWaveDividerWidget.h"

#include "Bugnauts/ProjectileSystem/Data/PatternData.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "WaveDividerWidget"

void SWaveDividerWidget::Construct(const FArguments& InArgs)
{
	SlotNumber = InArgs._SlotNumber;
	OnWaveDividerTimeChange = InArgs._OnWaveDividerTimeChange;
	WaveDividerList = InArgs._WaveDividerList;

	ConstructChildWidget();
}

void SWaveDividerWidget::ConstructChildWidget()
{
	ChildSlot[
		SNew(SOverlay)
		+ SOverlay::Slot()[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Graph.Node.Body"))
		]
		+ SOverlay::Slot()[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(5)[
				SNew(SImage)
		.Image(FAppStyle::GetBrush("Graph.Node.Body"))
		.ColorAndOpacity(FLinearColor(0, 0, 1, 1))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)[
				SNew(STextBlock)
				.Text(LOCTEXT("Time", "Time Between Waves"))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)[
				SNew(SNumericEntryBox<float>)

		.Value_Lambda([this]()-> float
				                             {
					                             if (SlotNumber >= (uint32) WaveDividerList.Get().Num()) return 0;
					                             FWaveDivider Divider = WaveDividerList.Get()[SlotNumber];
					                             return Divider.DistanceTime;
				                             })
		.OnValueCommitted_Lambda([this](float NewValue, ETextCommit::Type CommitType)
				                             {
					                             if (CommitType != ETextCommit::Type::OnEnter) return;
					                             bool DidRun = OnWaveDividerTimeChange.ExecuteIfBound(
						                             SlotNumber, NewValue);
					                             if (!DidRun)
					                             {
						                             UE_LOG(LogTemp, Warning, TEXT("Not bound somehow?"))
					                             }
				                             })

			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
