#pragma once
#include "CameraController.h"

class FPathEditorCameraController : public FEditorCameraController
{
public:

	
	
	virtual void UpdateSimulation(const FCameraControllerUserImpulseData& UserImpulseData, const float DeltaTime,
		const bool bAllowRecoilIfNoImpulse, const float MovementSpeedScale, FVector& InOutCameraPosition,
		FVector& InOutCameraEuler, float& InOutCameraFOV) override;
};
