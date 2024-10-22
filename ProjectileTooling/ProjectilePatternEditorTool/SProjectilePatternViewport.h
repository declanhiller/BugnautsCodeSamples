#pragma once
#include "SEditorViewport.h"

class UProjectileSpawnerController;

class SProjectilePatternViewport : public SEditorViewport
{
protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
public:
	void Construct(const FArguments& InArgs);
	void PlayPreviewPattern(class UPatternData* PatternData);

private:
	TSharedPtr<FPreviewScene> PreviewScene;

	TSharedPtr<class FProjectilePatternViewportClient> EditorViewportClient;
	
};
