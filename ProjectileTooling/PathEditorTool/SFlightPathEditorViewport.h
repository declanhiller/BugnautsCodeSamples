#pragma once
#include "FlightPathViewportCommands.h"
#include "SEditorViewport.h"
#include "SViewportToolBar.h"
#include "PathDataEditorToolkit.h"

class SFlightPathEditorViewport : public SEditorViewport
{
	SLATE_BEGIN_ARGS(SFlightPathEditorViewport)
		{
		}

	SLATE_ARGUMENT(TWeakPtr<FPathDataEditorToolkit>, PathDataEditor)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

protected:
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

public:
	void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);

	~SFlightPathEditorViewport();

	TSharedPtr<class FFlightPathEditorViewportClient> GetViewportClient() {return ViewportClient;}

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
	TSharedPtr<class FFlightPathEditorViewportClient> ViewportClient;

	FPreviewScene PreviewScene;

	FDelegateHandle PreviewFeatureLevelChangedHandle;

protected:
	virtual void BindCommands() override;

private:
	TWeakPtr<FPathDataEditorToolkit> PathDataEditor;
};

