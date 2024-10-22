#pragma once
#include "SFlightPathEditorViewport.h"

enum ESelectionObject
{
	NoneSelected, LeaveHandle, ArriveHandle, MainPoint, Segment
};

enum EToolType
{
	Selection, Add, Delete
};

struct HFlightPathKeyProxy : public HHitProxy
{

	DECLARE_HIT_PROXY()
	
	HFlightPathKeyProxy(uint32 InKeyIndex, ESelectionObject InType, EHitProxyPriority InPriority = HPP_Wireframe)
		:HHitProxy(InPriority)
	{
		KeyIndex = InKeyIndex;
		Type = InType;
	}

	uint32 KeyIndex;

	ESelectionObject Type;
};

struct HFlightPathSegmentProxy : public HHitProxy
{
	DECLARE_HIT_PROXY()

	HFlightPathSegmentProxy(int32 InStartKeyIndex, int32 InEndKeyIndex, EHitProxyPriority InPriority = HPP_Wireframe)
		:HHitProxy(InPriority)
	{
		StartKeyIndex = InStartKeyIndex;
		EndKeyIndex = InEndKeyIndex;
	}

	int32 StartKeyIndex;
	int32 EndKeyIndex;
};


struct FSelectionState
{
	//For Point Selection
private:
	int32 SelectedKeyIndex = INDEX_NONE;

	//For Segment Selection
	int32 StartSelectedSegmentKeyIndex = INDEX_NONE;

	int32 EndSelectedSegmentKeyIndex = INDEX_NONE;
public:
	//Selection Type
	ESelectionObject SelectedType = NoneSelected;

	void SetSelectedKey(int32 Index, ESelectionObject ObjectType)
	{
		StartSelectedSegmentKeyIndex = INDEX_NONE;
		EndSelectedSegmentKeyIndex = INDEX_NONE;
		SelectedType = ObjectType;

		SelectedKeyIndex = Index;
		
	}

	void SetSelectedSegment(int32 StartIndex, int32 EndIndex)
	{
		SelectedKeyIndex = INDEX_NONE;
		SelectedType = Segment;

		StartSelectedSegmentKeyIndex = StartIndex;
		EndSelectedSegmentKeyIndex = EndIndex;
	}

	int32 GetSelectedKeyIndex() const
	{
		return SelectedKeyIndex;
	}

	int32 GetStartSelectedSegmentKeyIndex() const
	{
		return StartSelectedSegmentKeyIndex;
	}

	int32 GetEndSelectedSegmentKeyIndex() const
	{
		return EndSelectedSegmentKeyIndex;
	}
	
};

class FFlightPathEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FFlightPathEditorViewportClient>
{
public:
	FFlightPathEditorViewportClient(TWeakPtr<FPathDataEditorToolkit> InPathDataEditor, FPreviewScene* InPreviewScene, const TSharedRef<SFlightPathEditorViewport>& InViewport);

	void InvalidatePreview(bool bResetCamera = true);
	
	void SwitchToSelectTool()
	{
		SetCurrentTool(Selection);
	}

	void SwitchToAddTool()
	{
		SetCurrentTool(Add);
	}

	void SwitchToDeleteTool()
	{
		SetCurrentTool(Delete);
	}


	UStaticMeshComponent* EditorFloorComp;
	UStaticMeshComponent* AnchorSphereComponent;
	
	FSceneView* StartView;
	FSceneViewFamily* StartViewFamily;

	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX,
		uint32 HitY) override;

	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	virtual FVector GetWidgetLocation() const override;

	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot,
		FVector& Scale) override;

	void SetCurrentTool(EToolType ToolType)
	{
		CurrentTool = ToolType;
	}

	EToolType GetCurrentTool()
	{
		return CurrentTool;
	}

protected:

	FSelectionState SelectionState;

	EToolType CurrentTool = Selection;

	
private:
	TWeakPtr<class FPathDataEditorToolkit> PathDataEditor;
};
