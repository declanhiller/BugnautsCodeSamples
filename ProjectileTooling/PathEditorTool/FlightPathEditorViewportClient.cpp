#include "FlightPathEditorViewportClient.h"

#include "CameraController.h"
#include "EditorModeManager.h"
#include "MouseDeltaTracker.h"
#include "PathEditorCameraController.h"
#include "UnrealEdGlobals.h"
#include "UnrealWidget.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "OodleDataCompression/Sdks/2.9.7/include/oodle2base.h"
#include "ThumbnailRendering/ThumbnailManager.h"

IMPLEMENT_HIT_PROXY(HFlightPathKeyProxy, HHitProxy);
IMPLEMENT_HIT_PROXY(HFlightPathSegmentProxy, HHitProxy);

FFlightPathEditorViewportClient::FFlightPathEditorViewportClient(TWeakPtr<FPathDataEditorToolkit> InPathDataEditor,
                                                                 FPreviewScene* InPreviewScene,
                                                                 const TSharedRef<SFlightPathEditorViewport>&
                                                                 InViewport)
	: FEditorViewportClient(nullptr, InPreviewScene, StaticCastSharedRef<SEditorViewport>(InViewport))
{
	PathDataEditor = InPathDataEditor;

	EngineShowFlags.DisableAdvancedFeatures();

	check(Widget);
	Widget->SetSnapEnabled(true);

	// Selectively set particular show flags that we need
	EngineShowFlags.SetSelectionOutline(GetDefault<ULevelEditorViewportSettings>()->bUseSelectionOutline);

	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowGrid;

	// now add floor
	EditorFloorComp = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("EditorFloorComp"));

	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/PhAT_FloorBox.PhAT_FloorBox"),
	                                                 NULL, LOAD_None, NULL);
	if (ensure(FloorMesh))
	{
		EditorFloorComp->SetStaticMesh(FloorMesh);
	}

	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaFloorMat.PersonaFloorMat"),
	                                            NULL, LOAD_None, NULL);
	if (ensure(Material))
	{
		EditorFloorComp->SetMaterial(0, Material);
	}

	EditorFloorComp->SetRelativeScale3D(FVector(3.f, 3.f, 1.f));
	EditorFloorComp->SetVisibility(GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowFloor);
	EditorFloorComp->SetCollisionEnabled(GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowFloor
		                                     ? ECollisionEnabled::QueryAndPhysics
		                                     : ECollisionEnabled::NoCollision);
	PreviewScene->AddComponent(EditorFloorComp, FTransform::Identity);

	// AnchorSphereComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("AnchorSphereComponent"));
	// UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/EditorCube.EditorCube"),
	//                                                   NULL, LOAD_None, NULL);
	// if(ensure(SphereMesh))
	// {
	// 	AnchorSphereComponent->SetStaticMesh(SphereMesh);
	// }
	//
	//
	// PreviewScene->AddComponent(AnchorSphereComponent, FTransform::Identity);


	// Turn off so that actors added to the world do not have a lifespan (so they will not auto-destroy themselves).
	PreviewScene->GetWorld()->bBegunPlay = false;

	PreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);


	//Toggle camera position and stuff
	ToggleOrbitCamera(false);


	CameraController = new FPathEditorCameraController();


	SetViewLocation(FVector(0.0f, 0.0f, 500.0f));
	SetViewRotation(FRotator(-90, 0, 0));

	ViewportType = ELevelViewportType::LVT_OrthoXY;
	SetViewMode(VMI_Lit);

	SelectionState = FSelectionState();
}

void FFlightPathEditorViewportClient::InvalidatePreview(bool bResetCamera)
{
	Invalidate();
}

void FFlightPathEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event,
                                                   uint32 HitX, uint32 HitY)
{
	if (!PathDataEditor.IsValid())
	{
		UE_LOG(LogSlate, Warning, TEXT("Path Data Editor pointer isn't valid"))
		return;
	}

	if (PathDataEditor.Pin()->GetPathData() == nullptr)
	{
		UE_LOG(LogSlate, Warning, TEXT("Path Data is null"))
		return;
	}

	UPathData* PathData = PathDataEditor.Pin()->GetPathData();

	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	if (HitProxy)
	{
		if (HitProxy->IsA(HFlightPathKeyProxy::StaticGetType()))
		{
			if (CurrentTool == Selection)
			{
				HFlightPathKeyProxy* KeyProxy = (HFlightPathKeyProxy*)HitProxy;
				SelectionState.SetSelectedKey(KeyProxy->KeyIndex, KeyProxy->Type);
				SelectionState.SelectedType = KeyProxy->Type;
				SetWidgetMode(UE::Widget::WM_Translate);
				return;
			}
			else if (CurrentTool == Delete)
			{
				PathData->Modify();
				HFlightPathKeyProxy* KeyProxy = (HFlightPathKeyProxy*)HitProxy;
				int32 Index = KeyProxy->KeyIndex;

				FInterpCurveVector& Spline = PathData->GetSplinePoints();
				Spline.Points.RemoveAt(Index);
				for (int i = Index; i < Spline.Points.Num(); i++)
				{
					Spline.Points[i].InVal -= 1.0f;
				}
				PathData->UpdateSpline();
				return;
			}
		}
		else if (HitProxy->IsA(HFlightPathSegmentProxy::StaticGetType()))
		{
			HFlightPathSegmentProxy* SegmentProxy = (HFlightPathSegmentProxy*)HitProxy;

			if (CurrentTool == Selection)
			{
				SelectionState.SetSelectedSegment(SegmentProxy->StartKeyIndex, SegmentProxy->EndKeyIndex);
				SetWidgetMode(UE::Widget::WM_None);
			}
			else if (CurrentTool == Add)
			{
				PathData->Modify();
				FInterpCurveVector& Spline = PathData->GetSplinePoints();

				float EvalIndex = (SegmentProxy->StartKeyIndex + SegmentProxy->EndKeyIndex) / 2.0f;
				FVector OutVal = Spline.Eval(EvalIndex, FVector(0));
				FInterpCurvePoint<FVector> NewPoint(SegmentProxy->EndKeyIndex,
				                                    OutVal,
				                                    FVector(100, 100, 0),
				                                    FVector(100, 100, 0),
				                                    CIM_CurveUser
				);

				Spline.Points.Insert(NewPoint, SegmentProxy->EndKeyIndex);

				for (int i = SegmentProxy->EndKeyIndex + 1; i < Spline.Points.Num(); i++)
				{
					Spline.Points[i].InVal += 1.0f;
				}

				PathData->UpdateSpline();

				return;
			}
		}
	}


	if (Click.GetKey() == EKeys::LeftMouseButton && CurrentTool == Add)
	{
		PathData->Modify();

		FInterpCurveVector& Spline = PathData->GetSplinePoints();

		const float InKey = Spline.Points.Num() ? Spline.Points.Last().InVal + 1.0f : 0.0f;
		Spline.Points.Emplace(InKey, Click.GetOrigin(), FVector(100, 100, 0), FVector(100, 100, 0), CIM_CurveUser);
		PathData->UpdateSpline();
	}
}

void FFlightPathEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	if (!PathDataEditor.IsValid())
	{
		UE_LOG(LogSlate, Warning, TEXT("Path Data Editor pointer isn't valid"))
		return;
	}

	if (PathDataEditor.Pin()->GetPathData() == nullptr)
	{
		UE_LOG(LogSlate, Warning, TEXT("Path Data is null"))
		return;
	}

	UPathData* PathData = PathDataEditor.Pin()->GetPathData();
	FInterpCurveVector& Spline = PathData->GetSplinePoints();
	//0, 1, 2


	// for(int i = 0; i < PathData->GetLength() - 1; i++)
	// {
	// 	FVector StartPoint = PathData->GetPoint(i).Location;
	// 	FVector EndPoint = PathData->GetPoint(i + 1).Location;
	// 	PDI->DrawLine(StartPoint, EndPoint, FLinearColor::Blue, 1, 2);
	// }

	FTransform OriginTransform = FTransform::Identity;

	FVector Origin = FVector::ZeroVector;
	int MaxRepeats = 10;
	for (int k = 0; k < MaxRepeats; k++)
	{
		if (k == 0)
		{
			for (int i = 0; i < Spline.Points.Num(); i++)
			{
				FVector PointPosition = PathData->SafeGetPointPosition(i).OutVal;

				FVector LeaveTangentPosition = PointPosition - Spline.Points[i].LeaveTangent;
				FVector ArriveTangentPosition = PointPosition + Spline.Points[i].ArriveTangent;


				PDI->SetHitProxy(NULL);

				PDI->DrawLine(LeaveTangentPosition, PointPosition, FLinearColor::Blue, SDPG_World, 2);
				PDI->DrawLine(ArriveTangentPosition, PointPosition, FLinearColor::Blue, SDPG_World, 2);

				PDI->SetHitProxy(new HFlightPathSegmentProxy(i - 1, i));
				if (i > 0)
				{
					const int32 Resolution = 25;
					FVector OldArcPos = Spline.Points[i - 1].OutVal;
					FLinearColor LineColor = SelectionState.SelectedType == Segment && SelectionState.
					                         GetEndSelectedSegmentKeyIndex() == i
						                         ? FLinearColor::Yellow
						                         : FLinearColor::Red;
					for (int32 Index = 1; Index <= Resolution; Index++)
					{
						const float Key = i - 1 + (Index / static_cast<float>(Resolution));
						const FVector NewArcPos = Spline.Eval(Key, FVector(0));
						PDI->DrawLine(OldArcPos, NewArcPos, LineColor, SDPG_World, 2);
						OldArcPos = NewArcPos;
					}
				}

				PDI->SetHitProxy(new HFlightPathKeyProxy(i, MainPoint));
				PDI->DrawPoint(PointPosition, FLinearColor::White, 20, SDPG_Foreground);

				PDI->SetHitProxy(new HFlightPathKeyProxy(i, LeaveHandle));
				PDI->DrawPoint(LeaveTangentPosition, FLinearColor::White, 20, SDPG_Foreground);

				PDI->SetHitProxy(new HFlightPathKeyProxy(i, ArriveHandle));
				PDI->DrawPoint(ArriveTangentPosition, FLinearColor::White, 20, SDPG_Foreground);

				if (i == Spline.Points.Num() - 1)
				{
					Origin = PointPosition;
				}
			}
		}
		else
		{
			for (int i = 0; i < Spline.Points.Num(); i++)
			{
				FVector PointPosition = PathData->SafeGetPointPosition(i).OutVal;

				PDI->SetHitProxy(NULL);

				if (i > 0)
				{
					const int32 Resolution = 25;
					FVector OldArcPos = Spline.Points[i - 1].OutVal;
					FLinearColor LineColor = FLinearColor::Red;
					for (int32 Index = 1; Index <= Resolution; Index++)
					{
						const float Key = i - 1 + (Index / static_cast<float>(Resolution));
						const FVector NewArcPos = Spline.Eval(Key, FVector(0));
						PDI->DrawLine(OldArcPos + Origin, NewArcPos + Origin, LineColor, SDPG_World, 2);
						OldArcPos = NewArcPos;
					}
				}

				if (i == Spline.Points.Num() - 1)
				{
					Origin += PointPosition;
				}
			}
		}

		FRotator OriginalRotation = OriginTransform.GetRotation().Rotator();
		// OriginalRotation.Add(PathData->RepeatingSettings.RotationOffset.X)
		
		// OriginTransform.SetRotation(OriginTransform.GetRotation())
	}
}


FVector FFlightPathEditorViewportClient::GetWidgetLocation() const
{
	if (!PathDataEditor.IsValid()) return FVector::ZeroVector;
	UPathData* PathData = PathDataEditor.Pin()->GetPathData();
	if (!PathData) return FVector::ZeroVector;
	FInterpCurveVector& Spline = PathData->GetSplinePoints();
	if (SelectionState.GetSelectedKeyIndex() == INDEX_NONE) return FVector::ZeroVector;
	if (SelectionState.GetSelectedKeyIndex() >= PathData->GetSplinePoints().Points.Num()) return FVector::ZeroVector;

	FInterpCurvePoint<FVector> Point = Spline.Points[SelectionState.GetSelectedKeyIndex()];

	FVector PointPosition = Point.OutVal;

	switch (SelectionState.SelectedType)
	{
	case LeaveHandle:
		return PointPosition - Point.LeaveTangent;
	case ArriveHandle:
		return PointPosition + Point.ArriveTangent;
	case MainPoint:
		return PathData->SafeGetPointPosition(SelectionState.GetSelectedKeyIndex()).OutVal;
	default:
		return FVector::ZeroVector;
	}
}

bool FFlightPathEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis,
                                                       FVector& Drag, FRotator& Rot, FVector& Scale)
{
	if (CurrentAxis == EAxisList::None) return false;

	if (Drag.IsZero()) return false;


	if (!PathDataEditor.IsValid()) return false;
	UPathData* PathData = PathDataEditor.Pin()->GetPathData();
	if (!PathData) return false;
	FInterpCurveVector& Spline = PathData->GetSplinePoints();
	if (SelectionState.GetSelectedKeyIndex() == INDEX_NONE) return false;
	if (SelectionState.GetSelectedKeyIndex() >= Spline.Points.Num()) return false;
	// || (SelectionState.GetSelectedKeyIndex() == 0 && SelectionState.SelectedType == MainPoint)

	PathData->Modify();

	FInterpCurvePoint<FVector>& Point = Spline.Points[SelectionState.GetSelectedKeyIndex()];

	const FVector NewDelta = SelectionState.SelectedType == LeaveHandle ? -Drag : Drag;
	const FVector Tangent = Point.LeaveTangent + NewDelta;
	switch (SelectionState.SelectedType)
	{
	case NoneSelected:
		return false;
	case LeaveHandle:

		Point.LeaveTangent = Tangent;
		Point.ArriveTangent = Tangent;
		PathData->UpdateSpline();
		break;
	case ArriveHandle:
		Point.LeaveTangent = Tangent;
		Point.ArriveTangent = Tangent;
		PathData->UpdateSpline();
		break;
	case MainPoint:
		Point.OutVal += Drag;
		PathData->UpdateSpline();
		break;
	default:
		return false;;
	}

	Invalidate();
	return true;
}
