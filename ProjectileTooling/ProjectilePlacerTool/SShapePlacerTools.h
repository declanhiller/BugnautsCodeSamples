#pragma once

// USTRUCT()
// struct FShape
// {
//
// 	FShape()
// 		:Factory(nullptr)
// 	{}
//
// 	FShape(UFactory* InFactory, const FAssetData& InAssetData)
// 		:Factory(InFactory)
// 		, AssetData(InAssetData)
// 	{
// 		AutoSetNativeAndDisplayName();
// 	}
//
// 	void AutoSetNativeAndDisplayName()
// 	{
// 		UClass* Class = AssetData.GetClass() == UClass::StaticClass() ? Cast<UClass>(AssetData.GetAsset()) : nullptr;
//
// 		if (Class)
// 		{
// 			Class->GetName(NativeName);
// 			DisplayName = Class->GetDisplayNameText();
// 		}
// 		else
// 		{
// 			NativeName = AssetData.AssetName.ToString();
// 			DisplayName = FText::FromName(AssetData.AssetName);
// 		}
// 	}
//
// 	FName GetNativeFName() const
// 	{ 
// 		if (NativeFName.IsNone() && !NativeName.IsEmpty())
// 		{
// 			NativeFName = FName(*NativeName);
// 		}
// 		return NativeFName;
// 	}
// 	
// 	UFactory* Factory;
// 	FAssetData AssetData;
// 	FString NativeName;
// 	FText DisplayName;
//
// private:
// 	mutable FName NativeFName;
// 	
// };

DECLARE_DELEGATE(FOnCreateSingleProjectile)
DECLARE_DELEGATE(FOnCreateCircle)
DECLARE_DELEGATE(FOnCreateLine)
DECLARE_DELEGATE(FOnCreateFan)

class SShapePlacerTools : public SCompoundWidget
{

	SLATE_BEGIN_ARGS(SShapePlacerTools)
	{
		
	}

	SLATE_EVENT(FOnCreateSingleProjectile, OnCreateSingleProjectile);
	SLATE_EVENT(FOnCreateCircle, OnCreateCircle);
	SLATE_EVENT(FOnCreateLine, OnCreateLine);
	SLATE_EVENT(FOnCreateFan, OnCreateFan);
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	
	FOnCreateSingleProjectile OnCreateSingleProjectile;
	FOnCreateCircle OnCreateCircle;
	FOnCreateLine OnCreateLine;
	FOnCreateFan OnCreateFan;
	

	FReply CreateSingleProjectile();
	FReply CreateCircle();
	FReply CreateLine();
	FReply CreateFan();
	
};
