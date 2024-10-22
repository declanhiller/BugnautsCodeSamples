#include "SWaveContainerWidget.h"

#include "Bugnauts/ProjectileSystem/Data/PatternData.h"

#define LOCTEXT_NAMESPACE "WaveContainerWidget"

void SWaveContainerWidget::Construct(const FArguments& InArgs)
{
	WaveContainerList = InArgs._WaveContainerList;
	SlotNumber = InArgs._SlotNumber;
	OnClick = InArgs._OnClick;

	ConstructChildWidget();
}

void SWaveContainerWidget::ConstructChildWidget()
{
	TSharedRef<SWidget> ChildWidget = SNew(SOverlay)
		+ SOverlay::Slot()[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Graph.Node.Body"))
		]
		+ SOverlay::Slot()[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			  .AutoHeight()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Top)[
				SNew(SOverlay)
				+ SOverlay::Slot()[
					SNew(SImage)
						.Image(FAppStyle::GetBrush("Graph.Node.TitleGloss"))
						.ColorAndOpacity(FLinearColor(1, 0, 0, 1))
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				[
					SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("Graph.Node.TitleHighlight"))
						.BorderBackgroundColor(FLinearColor(1, 0, 0, 1))
						.ColorAndOpacity(FLinearColor(1, 0, 0, 1))
					[
						SNew(SSpacer)
						.Size(FVector2D(20, 20))
					]
				]

				+ SOverlay::Slot()
				  .HAlign(HAlign_Center)
				  .VAlign(VAlign_Center)[
					SNew(STextBlock)
					.Text_Lambda([this]() -> FText
					{
						if (SlotNumber >= (uint32) WaveContainerList.Get().Num()) return LOCTEXT("Test", "Empty");
						FWaveContainer WaveContainer = WaveContainerList.Get()[SlotNumber];
						return WaveContainer.WaveData == nullptr
							       ? LOCTEXT("Test", "Empty")
							       : FText::FromName(WaveContainer.WaveData->GetFName());
					})
				]


			]
			+ SVerticalBox::Slot()[
				ConstructChildWidgetContent()
			]
		];

	ChildSlot[
		ChildWidget
	];
}

TSharedRef<SWidget> SWaveContainerWidget::ConstructChildWidgetContent()
{
	TSharedRef<SVerticalBox> WidgetContent = SNew(SVerticalBox);


	//Projectile number display
	WidgetContent->AddSlot()[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()[
			SNew(STextBlock)
			.Text(LOCTEXT("ProjectileNumber", "Number of Projectiles"))
		]
		+ SHorizontalBox::Slot()[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				if (SlotNumber >= (uint32) WaveContainerList.Get().Num()) return LOCTEXT("Test", "Empty");
				UWaveData* WaveData = WaveContainerList.Get()[SlotNumber].WaveData;
				if (!WaveData) return LOCTEXT("Nothing", "Nothing");
				return FText::AsNumber(WaveData->ProjectileData.Num());
			})
		]
	];


	// TSharedRef<SHorizontalBox> ProjectileSpawnRadiusDisplayWidget = SNew(SHorizontalBox);
	//
	//
	// if (WaveContainerList.Get()[SlotNumber].WaveData->RadiusToSpawnProjectilesAt.TypeOfProperty ==
	// 	ETypeOfProperty::SingleValue)
	// {
	// 	ProjectileSpawnRadiusDisplayWidget->AddSlot()[
	// 		SNew(STextBlock)
	// 		.Text_Lambda([this]()-> FText
	// 		{
	// 			UWaveData* WaveData = WaveContainerList.Get()[SlotNumber].WaveData;
	// 			if (!WaveData) return LOCTEXT("Nothing", "Nothing");
	// 			return FText::AsNumber(WaveData->RadiusToSpawnProjectilesAt.Value);
	// 		})
	// 	];
	// }
	// else
	// {
	// 	ProjectileSpawnRadiusDisplayWidget->AddSlot()[
	// 		SNew(STextBlock)
	// 		.Text_Lambda([this]()-> FText
	// 		{
	// 			UWaveData* WaveData = WaveContainerList.Get()[SlotNumber].WaveData;
	// 			if (!WaveData) return LOCTEXT("Nothing", "Nothing");
	// 			return FText::AsNumber(WaveData->RadiusToSpawnProjectilesAt.MinValue);
	// 		})
	// 	];
	// 	ProjectileSpawnRadiusDisplayWidget->AddSlot()[
	// 		SNew(STextBlock)
	// 		.Text(LOCTEXT("to", "to"))
	// 	];
	// 	ProjectileSpawnRadiusDisplayWidget->AddSlot()[
	// 		SNew(STextBlock)
	// 		.Text_Lambda([this]()-> FText
	// 		{
	// 			UWaveData* WaveData = WaveContainerList.Get()[SlotNumber].WaveData;
	// 			if (!WaveData) return LOCTEXT("Nothing", "Nothing");
	// 			return FText::AsNumber(WaveData->RadiusToSpawnProjectilesAt.MaxValue);
	// 		})
	// 	];
	// }
	//
	//
	// WidgetContent->AddSlot()[
	// 	SNew(SHorizontalBox)
	// 	+ SHorizontalBox::Slot()[
	// 		SNew(STextBlock)
	// 		.Text(LOCTEXT("ProjectileSpawnRadius", "Projectile Spawn Radius"))
	// 	]
	// 	+ SHorizontalBox::Slot()[
	// 		ProjectileSpawnRadiusDisplayWidget
	// 	]
	// ];

	return WidgetContent;
}

FReply SWaveContainerWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);

	OnClick.Execute(SlotNumber);
	
	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
