#include "SShapePlacerTools.h"

#define LOCTEXT_NAMESPACE "SShapePlacerTools"

void SShapePlacerTools::Construct(const FArguments& InArgs)
{

	OnCreateSingleProjectile = InArgs._OnCreateSingleProjectile;
	OnCreateCircle = InArgs._OnCreateCircle;
	OnCreateFan = InArgs._OnCreateFan;
	OnCreateLine = InArgs._OnCreateLine;

	ChildSlot[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SButton)
			.Text(LOCTEXT("Single", "Single Projectile"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(this, &SShapePlacerTools::CreateSingleProjectile)
		]
		+ SVerticalBox::Slot()
		[
			SNew(SButton)
			.Text(LOCTEXT("Circle", "Circle Shape"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(this, &SShapePlacerTools::CreateCircle)
		]
		+ SVerticalBox::Slot()
		[
			SNew(SButton)
			.Text(LOCTEXT("Fan", "Fan Shape"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(this, &SShapePlacerTools::CreateFan)
		]
		+ SVerticalBox::Slot()
		[
			SNew(SButton)
			.Text(LOCTEXT("Line", "Line Shape"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(this, &SShapePlacerTools::CreateLine)
		]
	];
}

FReply SShapePlacerTools::CreateSingleProjectile()
{
	OnCreateSingleProjectile.ExecuteIfBound();
	return FReply::Handled();
}

FReply SShapePlacerTools::CreateCircle()
{
	OnCreateCircle.ExecuteIfBound();
	return FReply::Handled();
}

FReply SShapePlacerTools::CreateLine()
{
	OnCreateLine.ExecuteIfBound();
	return FReply::Handled();
}

FReply SShapePlacerTools::CreateFan()
{
	OnCreateFan.ExecuteIfBound();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
