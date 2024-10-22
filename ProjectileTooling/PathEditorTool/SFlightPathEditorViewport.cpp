#include "SFlightPathEditorViewport.h"

#include "EditorViewportCommands.h"
#include "FlightPathEditorViewportClient.h"
#include "FlightPathViewportCommands.h"
#include "SEditorViewportToolBarMenu.h"
#include "STransformViewportToolbar.h"
#include "SViewportToolBar.h"
#include "OodleDataCompression/Sdks/2.9.6/include/oodle2base.h"

#define LOCTEXT_NAMESPACE "FlightPathEditorViewport"

class SFlightPathEditorViewportToolBar : public SViewportToolBar
{
	SLATE_BEGIN_ARGS(SFlightPathEditorViewportToolBar)
		{
		}

		SLATE_ARGUMENT(TWeakPtr<SFlightPathEditorViewport>, EditorViewport)

	SLATE_END_ARGS()



	void Construct(const FArguments& InArgs)
	{
		EditorViewport = InArgs._EditorViewport;

		const FMargin ToolbarSlotPadding(4.0f, 1.0f);

		this->ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("EditorViewportToolBar.Background"))
			.Cursor(EMouseCursor::Default)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .Padding(ToolbarSlotPadding)
				[
					SNew(SEditorViewportToolbarMenu)
					.ParentToolBar(SharedThis(this))
					.Label(this, &SFlightPathEditorViewportToolBar::GetCameraMenuLabel)
					.OnGetMenuContent(this, &SFlightPathEditorViewportToolBar::GenerateCameraMenu)
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .Padding(ToolbarSlotPadding)
				[
					SNew(SEditorViewportToolbarMenu)
					.ParentToolBar(SharedThis(this))
					.Cursor(EMouseCursor::Default)
					.Label(this, &SFlightPathEditorViewportToolBar::GetViewMenuLabel)
					.OnGetMenuContent(this, &SFlightPathEditorViewportToolBar::GenerateViewMenu)
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(ToolbarSlotPadding)
				[
					SNew(SEditorViewportToolbarMenu)
					.ParentToolBar(SharedThis(this))
					.Cursor(EMouseCursor::Default)
					.Label(this, &SFlightPathEditorViewportToolBar::GetToolMenuLabel)
					.OnGetMenuContent(this, &SFlightPathEditorViewportToolBar::GenerateToolMenu)
				]
				+ SHorizontalBox::Slot()
				  .Padding(ToolbarSlotPadding)
				  .HAlign(HAlign_Right)
				[
					SNew(STransformViewportToolBar)
					.Viewport(EditorViewport.Pin().ToSharedRef())
					.CommandList(EditorViewport.Pin()->GetCommandList())
				]
			]
		];

		SViewportToolBar::Construct(SViewportToolBar::FArguments());
	}


	FText GetCameraMenuLabel() const
	{
		if (EditorViewport.IsValid())
		{
			return GetCameraMenuLabelFromViewportType(EditorViewport.Pin()->GetViewportClient()->GetViewportType());
		}

		return NSLOCTEXT("BlueprintEditor", "CameraMenuTitle_Default", "Camera");
	}


	TSharedRef<SWidget> GenerateCameraMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid()
			                                               ? EditorViewport.Pin()->GetCommandList()
			                                               : nullptr;

		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder CameraMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);

		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Perspective);

		CameraMenuBuilder.BeginSection("LevelViewportCameraType_Ortho",
		                               NSLOCTEXT("BlueprintEditor", "CameraTypeHeader_Ortho", "Orthographic"));
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
		CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
		CameraMenuBuilder.EndSection();

		return CameraMenuBuilder.MakeWidget();
	}

	FText GetToolMenuLabel() const
	{
		switch (EToolType CurrentTool = EditorViewport.Pin()->GetViewportClient()->GetCurrentTool()) {
		case Selection:
			return LOCTEXT("selection", "Selection");
		case Add:
			return LOCTEXT("add", "Add Points");
		case Delete:
			return LOCTEXT("delete", "Delete Points");
		}
		return LOCTEXT("default", "Default");
		
	};

	TSharedRef<SWidget> GenerateToolMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid()
												   ? EditorViewport.Pin()->GetCommandList()
												   : nullptr;

		FMenuBuilder ToolMenuBuilder(true, CommandList);

		ToolMenuBuilder.AddMenuEntry(FFlightPathViewportCommands::Get().SelectCommand);
		ToolMenuBuilder.AddMenuEntry(FFlightPathViewportCommands::Get().AddCommand);
		ToolMenuBuilder.AddMenuEntry(FFlightPathViewportCommands::Get().DeleteCommand);


		// ToolMenuBuilder.BeginSection("LevelViewportCameraType_Ortho",
		// 							   NSLOCTEXT("BlueprintEditor", "CameraTypeHeader_Ortho", "Orthographic"));
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
		// ToolMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
		// ToolMenuBuilder.EndSection();

		return ToolMenuBuilder.MakeWidget();
	}

	FText GetViewMenuLabel() const
	{
		FText Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Default", "View");

		if (EditorViewport.IsValid())
		{
			switch (EditorViewport.Pin()->GetViewportClient()->GetViewMode())
			{
			case VMI_Lit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Lit", "Lit");
				break;

			case VMI_Unlit:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Unlit", "Unlit");
				break;

			case VMI_BrushWireframe:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_Wireframe", "Wireframe");
				break;

			case VMI_CollisionVisibility:
				Label = NSLOCTEXT("BlueprintEditor", "ViewMenuTitle_CollisionVisibility", "Collision Visibility");
				break;
			}
		}

		return Label;
	}

	TSharedRef<SWidget> GenerateViewMenu() const
	{
		TSharedPtr<const FUICommandList> CommandList = EditorViewport.IsValid()
			                                               ? EditorViewport.Pin()->GetCommandList()
			                                               : nullptr;

		const bool bInShouldCloseWindowAfterMenuSelection = true;
		FMenuBuilder ViewMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList);

		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().LitMode, NAME_None,
		                             NSLOCTEXT("BlueprintEditor", "LitModeMenuOption", "Lit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().UnlitMode, NAME_None,
		                             NSLOCTEXT("BlueprintEditor", "UnlitModeMenuOption", "Unlit"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().WireframeMode, NAME_None,
		                             NSLOCTEXT("BlueprintEditor", "WireframeModeMenuOption", "Wireframe"));
		ViewMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().CollisionVisibility, NAME_None,
		                             NSLOCTEXT("BlueprintEditor", "CollisionVisibilityMenuOption",
		                                       "Visibility Collision"));

		return ViewMenuBuilder.MakeWidget();
	}

private:
	/** Reference to the parent viewport */
	TWeakPtr<SFlightPathEditorViewport> EditorViewport;
};

void SFlightPathEditorViewport::Construct(const FArguments& InArgs)
{
	PathDataEditor = InArgs._PathDataEditor;

	SEditorViewport::Construct(SEditorViewport::FArguments());

	if (ViewportClient.IsValid())
	{
		UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
		if (World != nullptr)
		{
			World->ChangeFeatureLevel(GWorld->FeatureLevel);
		}
	}

	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda(
		[this](ERHIFeatureLevel::Type NewFeatureLevel)
		{
			if (ViewportClient.IsValid())
			{
				UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
				if (World != nullptr)
				{
					World->ChangeFeatureLevel(NewFeatureLevel);

					// Refresh the preview scene. Don't change the camera.
					RequestRefresh(false);
				}
			}
		});
	
	// Refresh the preview scene
	RequestRefresh(true);
}

TSharedPtr<SWidget> SFlightPathEditorViewport::MakeViewportToolbar()
{
	return SNew(SFlightPathEditorViewportToolBar).EditorViewport(SharedThis(this))
	.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());;
}

void SFlightPathEditorViewport::RequestRefresh(bool bResetCamera, bool bRefreshNow)
{
	if (bRefreshNow)
	{
		if (ViewportClient.IsValid())
		{
			ViewportClient->InvalidatePreview(bResetCamera);
		}
	}
	else
	{
	}
}

SFlightPathEditorViewport::~SFlightPathEditorViewport()
{
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);

	if (ViewportClient.IsValid())
	{
		// Reset this to ensure it's no longer in use after destruction
		ViewportClient->Viewport = NULL;
	}
}

TSharedRef<FEditorViewportClient> SFlightPathEditorViewport::MakeEditorViewportClient()
{
	ViewportClient =
		MakeShareable(new FFlightPathEditorViewportClient(PathDataEditor, &PreviewScene, SharedThis(this)));

	return ViewportClient.ToSharedRef();
}

void SFlightPathEditorViewport::BindCommands()
{

	FFlightPathViewportCommands::Register();
	
	SEditorViewport::BindCommands();

	const FFlightPathViewportCommands& Commands = FFlightPathViewportCommands::Get();
	

	
	CommandList->MapAction(
		Commands.SelectCommand,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FFlightPathEditorViewportClient::SwitchToSelectTool));

	CommandList->MapAction(
		Commands.AddCommand,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FFlightPathEditorViewportClient::SwitchToAddTool));

	CommandList->MapAction(
		Commands.DeleteCommand,
		FExecuteAction::CreateSP(ViewportClient.Get(), &FFlightPathEditorViewportClient::SwitchToDeleteTool));

	
}

#undef LOCTEXT_NAMESPACE
