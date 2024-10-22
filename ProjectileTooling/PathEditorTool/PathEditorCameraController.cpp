#include "PathEditorCameraController.h"

void FPathEditorCameraController::UpdateSimulation(const FCameraControllerUserImpulseData& UserImpulseData,
	const float DeltaTime, const bool bAllowRecoilIfNoImpulse, const float MovementSpeedScale,
	FVector& InOutCameraPosition, FVector& InOutCameraEuler, float& InOutCameraFOV)
{
	// FEditorCameraController::UpdateSimulation(UserImpulseData, DeltaTime, bAllowRecoilIfNoImpulse, MovementSpeedScale,
	//                                           InOutCameraPosition,
	//                                           InOutCameraEuler, InOutCameraFOV);

	// InOutCameraEuler = FVector(0, 0, 0);
	
}
