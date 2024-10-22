#pragma once

struct FGridInfo {

	FGridInfo(float MinX, float MaxX, float MinY, float MaxY, const FVector2D WidgetSize)
	{
		
	}
	
};


class SProjectilePathEditor : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SProjectilePathEditor)
		{
		}

		SLATE_ATTRIBUTE(float, ViewMinInput)
		SLATE_ATTRIBUTE(float, ViewMaxInput)
		SLATE_ATTRIBUTE(float, ViewMinOutput)
		SLATE_ATTRIBUTE(float, ViewMaxOutput)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;
};
