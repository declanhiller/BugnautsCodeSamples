#include "SProjectilePathEditor.h"

void SProjectilePathEditor::Construct(const FArguments& InArgs)
{
	
}

int32 SProjectilePathEditor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{

	float Height = AllottedGeometry.Size.Y;
	
	//draw background
	int32 BackgroundLayerId = LayerId;

	ESlateDrawEffect DrawEffect = ESlateDrawEffect::None;
	
	FSlateDrawElement::MakeBox(OutDrawElements, BackgroundLayerId, AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("ToolPanel.GroupBorder"), DrawEffect, FLinearColor(0.8f, 0.8f, 0.8f));

	

	return LayerId + 1;
}
