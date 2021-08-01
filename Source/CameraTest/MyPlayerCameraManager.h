// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MyPlayerCameraManager.generated.h"

class UMyCameraAnimInst;
class UCameraAnim;
/**
 * 
 */
UCLASS()
class CAMERATEST_API AMyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    virtual class UMyCameraAnimInst* PlayCustomCameraAnim(class UCameraAnim* Anim, float Rate = 1.f, float Scale = 1.f, float BlendInTime = 0.f, float BlendOutTime = 0.f,
		                                                  bool bLoop = false, bool bRandomStartTime = false, float Duration = 0.f, 
		                                                  ECameraAnimPlaySpace::Type PlaySpace = ECameraAnimPlaySpace::CameraLocal, 
		                                                  FRotator UserPlaySpaceRot = FRotator::ZeroRotator, 
		                                                  bool bTargetPawn = false);
public:
	virtual void PostInitializeComponents() override;
	virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
};
