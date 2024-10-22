#pragma once

#include "WaveDataV2EditorToolkit.h"
#include "Bugnauts/ProjectileSystem/Data/WaveShapes/WaveShape.h"

class SWaveShapeEditorViewer;

class PROJECTILEPATTERNEDITOR_API FWaveEditorListNode : public TSharedFromThis<FWaveEditorListNode>
{
public:
	explicit FWaveEditorListNode(UWaveShape* InShape);

	~FWaveEditorListNode() = default;

	UWaveShape* GetObject(bool bEvenIfPendingKill = false);
	

	FString GetDisplayString() const;

	FText GetDisplayText() const;


	void SetOngoingCreateTransaction(TUniquePtr<FScopedTransaction> InTransaction);
	void CloseOngoingCreateTransaction();
	void GetOngoingCreateTransaction(TUniquePtr<FScopedTransaction>& OutPtr) {OutPtr = MoveTemp(OngoingCreateTransaction);}
	bool HasOngoingTransaction() const {return OngoingCreateTransaction.IsValid();}

protected:

	UWaveShape* WaveShape;
	
	TUniquePtr<FScopedTransaction> OngoingCreateTransaction;


	
};

class SWaveShapeEditorViewer : public SCompoundWidget
{

	SLATE_BEGIN_ARGS(SWaveShapeEditorViewer) {}
	
	SLATE_ARGUMENT(TWeakPtr<FWaveDataV2EditorToolkit>, WaveDataEditor)

	SLATE_END_ARGS()
	

	void Construct(const FArguments& InArgs);

	FReply ButtonPressed();
	
	DECLARE_DELEGATE_OneParam(FOnSelectionUpdated, TSharedPtr<FWaveEditorListNode>);

	void AddShape(UWaveShape* Shape);
	void RemoveShape(UWaveShape* Shape);

	TArray<TSharedPtr<FWaveEditorListNode>> List;

	void PopulateContextMenu(UToolMenu* Menu);

	void GetSelectedItemsForContextMenu(TArray<UWaveShape*>& OutSelectedItems) const;

protected:

	TWeakPtr<FWaveDataV2EditorToolkit> WaveDataEditor;
	
	FOnSelectionUpdated OnSelectionUpdated;

	TAttribute<UObject*> ObjectContext;

	TSharedRef<ITableRow> MakeTableRowWidget(TSharedPtr<FWaveEditorListNode> InNodePtr, const TSharedRef<STableViewBase>& OwnerTable);

	void NewSelection(TSharedPtr<FWaveEditorListNode> NewSelectedNode, ESelectInfo::Type SelectInfo);
	
	TSharedPtr<SListView<TSharedPtr<FWaveEditorListNode>>> ListViewWidget;
	
	TSharedPtr<SWidget> CreateContextMenu();

	void RegisterContextMenu();

	TSharedPtr<FUICommandList> CommandList;

	virtual void CreateCommandList();

	void DeleteWaveShape();

	void ConvertWaveShape();

	
	
	
};
