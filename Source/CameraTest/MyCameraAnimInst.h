// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraAnimInst.h"
#include "MyCameraAnimInst.generated.h"

/**
 * 
 */
UCLASS()
class CAMERATEST_API UMyCameraAnimInst : public UCameraAnimInst
{
	GENERATED_BODY()
	
public:
	void SetIsTargetPawnSpace(bool bTargetPawn) {
		bTargetPawnSpace = bTargetPawn;
	};

	bool PorcessTargetPawnCameraAnim(class ACameraActor const* AnimatedCamActor, class UCameraAnimInst const* AnimInst, FMinimalViewInfo& InOutPOV);
private:
	bool bTargetPawnSpace = false;
};
