#include "SWaveShapeEditorViewer.h"

#include "BlueprintEditor.h"
#include "FlightPathViewportCommands.h"
#include "SubobjectData.h"
#include "SubobjectDataSubsystem.h"
#include "WaveEditorCommands.h"
#include "WaveEditorMenuContext.h"
#include "Bugnauts/ProjectileSystem/Data/WaveShapes/SingleProjectile.h"
#include "Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"
#include "Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "WaveShapeEditorViewer"

static const FName WaveEditorMenuContextName("WaveEditorContextMenu");


FWaveEditorListNode::FWaveEditorListNode(UWaveShape* InShape)
	: WaveShape(InShape)
{
	check(IsValid(InShape));
}

UWaveShape* FWaveEditorListNode::GetObject(bool bEvenIfPendingKill)
{
	return WaveShape;
}

FString FWaveEditorListNode::GetDisplayString() const
{
	return WaveShape ? WaveShape->GetClass()->GetName() : TEXT("UNKNOWN");
}

FText FWaveEditorListNode::GetDisplayText() const
{
	return FText::FromString(GetDisplayString());
}

void FWaveEditorListNode::SetOngoingCreateTransaction(TUniquePtr<FScopedTransaction> InTransaction)
{
	OngoingCreateTransaction = MoveTemp(InTransaction);
}

void FWaveEditorListNode::CloseOngoingCreateTransaction()
{
	OngoingCreateTransaction.Reset();
}



void SWaveShapeEditorViewer::Construct(const FArguments& InArgs)
{
	WaveDataEditor = InArgs._WaveDataEditor;

	CreateCommandList();
	
	ChildSlot[
		SNew(SScrollBox)
		+ SScrollBox::Slot()[
			SAssignNew(ListViewWidget, SListView<TSharedPtr<FWaveEditorListNode>>)
			.ItemHeight(24)
			.ListItemsSource(&List)
			.OnGenerateRow(this, &SWaveShapeEditorViewer::MakeTableRowWidget)
			.OnSelectionChanged(this, &SWaveShapeEditorViewer::NewSelection)
			.OnContextMenuOpening(this, &SWaveShapeEditorViewer::CreateContextMenu)
		]
	];
}

FReply SWaveShapeEditorViewer::ButtonPressed()
{
	List.Add(MakeShareable(new FWaveEditorListNode(WaveDataEditor.Pin()->GetWaveData()->Shapes[0])));

	ListViewWidget->RequestListRefresh();

	return FReply::Handled();
}

void SWaveShapeEditorViewer::AddShape(UWaveShape* Shape)
{
	TSharedPtr<FWaveEditorListNode> Node = MakeShareable(new FWaveEditorListNode(Shape));
	List.Add(Node);
	ListViewWidget->RequestListRefresh();
}

void SWaveShapeEditorViewer::RemoveShape(UWaveShape* Shape)
{
	TArray<TSharedPtr<FWaveEditorListNode>> SelectedItems = ListViewWidget->GetSelectedItems();
	for(int i = 0; i < SelectedItems.Num(); i++)
	{
		if(SelectedItems[i]->GetObject() == Shape)
		{
			List.Remove(SelectedItems[i]);
		}
	}
	ListViewWidget->RequestListRefresh();
}

void SWaveShapeEditorViewer::PopulateContextMenu(UToolMenu* Menu)
{
	TArray<TSharedPtr<FWaveEditorListNode>> SelectedItems = ListViewWidget->GetSelectedItems();
	if(SelectedItems.Num() <= 0) return;
	TSharedPtr<FWaveEditorListNode> SelectedItem = SelectedItems[0];
	FToolMenuSection& Section = Menu->AddSection("Basic", LOCTEXT("BasicHeading", "Basic Actions"));
	
	TWeakPtr<SWaveShapeEditorViewer> WeakEditorPtr;
	if (UWaveEditorMenuContext* MenuContext = Menu->FindContext<UWaveEditorMenuContext>())
	{
		WeakEditorPtr = MenuContext->WaveEditor;
	}

	Section.AddMenuEntry(FWaveEditorCommands::Get().ConvertCommand);
	Section.AddMenuEntry(FWaveEditorCommands::Get().DeleteCommand);

}

void SWaveShapeEditorViewer::GetSelectedItemsForContextMenu(TArray<UWaveShape*>& OutSelectedItems) const
{
	TArray<TSharedPtr<FWaveEditorListNode>> Items = ListViewWidget->GetSelectedItems();
	for (TSharedPtr<FWaveEditorListNode> Node : Items)
	{
		OutSelectedItems.Add(Node->GetObject());
	}
}

TSharedRef<ITableRow> SWaveShapeEditorViewer::MakeTableRowWidget(TSharedPtr<FWaveEditorListNode> InNodePtr,
                                                                 const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedRef<STableRow<TSharedPtr<FWaveEditorListNode>>> Row = SNew(STableRow<TSharedPtr<FWaveEditorListNode>>,
	                                                                  OwnerTable)
		.Padding(2.f)[
			SNew(STextBlock).Text(InNodePtr->GetDisplayText())
		];
	return Row;
}

void SWaveShapeEditorViewer::NewSelection(TSharedPtr<FWaveEditorListNode> NewSelectedNode, ESelectInfo::Type SelectInfo)
{
	if(NewSelectedNode == nullptr) return;
	WaveDataEditor.Pin()->SetNewSelection(NewSelectedNode->GetObject());
}


TSharedPtr<SWidget> SWaveShapeEditorViewer::CreateContextMenu()
{
	TArray<TSharedPtr<FWaveEditorListNode>> SelectedItems = ListViewWidget->GetSelectedItems();

	if (SelectedItems.Num() > 0)
	{
		RegisterContextMenu();
		UWaveEditorMenuContext* ContextObject = NewObject<UWaveEditorMenuContext>();
		ContextObject->WaveEditor = SharedThis(this);
		FToolMenuContext ToolMenuContext(CommandList, TSharedPtr<FExtender>(), ContextObject);
		return UToolMenus::Get()->GenerateWidget(WaveEditorMenuContextName, ToolMenuContext);
	}
	return TSharedPtr<SWidget>();
	
}

void SWaveShapeEditorViewer::RegisterContextMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus->IsMenuRegistered(WaveEditorMenuContextName))
	{
		UToolMenu* Menu = ToolMenus->RegisterMenu(WaveEditorMenuContextName);
		Menu->AddDynamicSection("BasicActions", FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
		{
			UWaveEditorMenuContext* ContextObject = InMenu->FindContext<UWaveEditorMenuContext>();
			if (ContextObject && ContextObject->WaveEditor.IsValid())
			{
				ContextObject->WaveEditor.Pin()->PopulateContextMenu(InMenu);
			}
		}));
	}
}

void SWaveShapeEditorViewer::CreateCommandList()
{
	CommandList = MakeShareable(new FUICommandList);

	FWaveEditorCommands::Register();

	CommandList->MapAction(FWaveEditorCommands::Get().ConvertCommand,
		FUIAction(FExecuteAction::CreateSP(this, &SWaveShapeEditorViewer::ConvertWaveShape),
			FCanExecuteAction()));

	CommandList->MapAction(FWaveEditorCommands::Get().DeleteCommand,
		FUIAction(FExecuteAction::CreateSP(this, &SWaveShapeEditorViewer::DeleteWaveShape),
			FCanExecuteAction()));
	
}

void SWaveShapeEditorViewer::DeleteWaveShape()
{
	const FScopedTransaction Transaction( LOCTEXT("DeleteShape", "Delete Shape") );
	WaveDataEditor.Pin()->GetWaveData()->Modify();
	TArray<TSharedPtr<FWaveEditorListNode>> OutNodes;
	ListViewWidget->GetSelectedItems(OutNodes);
	if(OutNodes.Num() <= 0) return;
	TSharedPtr<FWaveEditorListNode> SelectedNode = OutNodes[0];
	List.Remove(SelectedNode);
	UWaveShape* ShapeToDelete = SelectedNode->GetObject();
	WaveDataEditor.Pin()->GetWaveData()->Shapes.Remove(ShapeToDelete);
	WaveDataEditor.Pin()->NotifyForChanges();
	ListViewWidget->RequestListRefresh();
}

void SWaveShapeEditorViewer::ConvertWaveShape()
{
	const FScopedTransaction Transaction( LOCTEXT("WaveShape", "Wave Shape") );
	WaveDataEditor.Pin()->GetWaveData()->Modify();
	TArray<TSharedPtr<FWaveEditorListNode>> OutNodes;
	ListViewWidget->GetSelectedItems(OutNodes);
	if(OutNodes.Num() <= 0) return;
	TSharedPtr<FWaveEditorListNode> SelectedNode = OutNodes[0];
	UWaveShape* Shape = SelectedNode->GetObject();
	if(Cast<USingleProjectile>(Shape)) return; //no need to convert a single projectile
	TArray<FWaveProjectileContainer> Containers = Shape->CalculateProjectilesFromShape();
	for(int i = 0; i < Containers.Num(); i++)
	{
		USingleProjectile* SingleProj = NewObject<USingleProjectile>(WaveDataEditor.Pin()->GetWaveData());
		SingleProj->LocalPosition = Containers[i].LocalPositionToSpawn;
		SingleProj->LocalDirection = Containers[i].LocalDirectionToFire;
		SingleProj->DirectionLogic = ESingleProjectileDirectionLogic::SingleSpecifiedDirection;
		SingleProj->ProjectileData = Containers[i].ProjectileData;
		SingleProj->PathData = Containers[i].PathData;
		WaveDataEditor.Pin()->GetWaveData()->Shapes.Add(SingleProj);
		AddShape(SingleProj);
	}
	RemoveShape(Shape);
	WaveDataEditor.Pin()->GetWaveData()->Shapes.Remove(Shape);
	WaveDataEditor.Pin()->NotifyForChanges();
}

#undef LOCTEXT_NAMESPACE
