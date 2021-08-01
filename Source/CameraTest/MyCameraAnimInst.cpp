// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCameraAnimInst.h"
#include "Kismet/GameplayStatics.h"
#include "Matinee/InterpGroupInst.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraAnim.h"
#include "Camera/CameraComponent.h"

bool UMyCameraAnimInst::PorcessTargetPawnCameraAnim(class ACameraActor const* AnimatedCamActor, class UCameraAnimInst const* AnimInst, FMinimalViewInfo& InOutPOV)
{
	if (!bTargetPawnSpace)
	{
		return false;
	}

	float Scale = CurrentBlendWeight;

	//todo 根据项目需要替换成传入的
	APawn* CurTargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (CurrentBlendWeight > 0.f)
	{
		if (AnimatedCamActor)
		{
			FVector TargetLocation = AnimatedCamActor->GetActorLocation();
			InOutPOV.Location = FMath::Lerp(InOutPOV.Location, CurTargetPawn->GetActorTransform().TransformPosition(TargetLocation), Scale);
			FRotator TargetRotation = AnimatedCamActor->GetActorRotation();
			InOutPOV.Rotation = FMath::Lerp(InOutPOV.Rotation, CurTargetPawn->GetActorTransform().TransformRotation(FQuat(TargetRotation)).Rotator(), Scale);
		}
	}

    // fov
    if (bHasFOVTrack)
    {
        const float FOVMin = 5.f;
        const float FOVMax = 170.f;

        // Interp the FOV toward the camera component's FOV based on Scale
        if (CamAnim->bRelativeToInitialFOV)
        {
            InOutPOV.FOV += (AnimatedCamActor->GetCameraComponent()->FieldOfView - InitialFOV) * Scale;
        }
        else
        {
            const int32 DesiredDirection = FMath::Sign(AnimatedCamActor->GetCameraComponent()->FieldOfView - InOutPOV.FOV);
            const int32 InitialDirection = FMath::Sign(AnimatedCamActor->GetCameraComponent()->FieldOfView - InitialFOV);
            if (DesiredDirection != InitialDirection)
            {
                InOutPOV.FOV = FMath::Clamp(InOutPOV.FOV + ((AnimatedCamActor->GetCameraComponent()->FieldOfView - InOutPOV.FOV) * Scale), InOutPOV.FOV, AnimatedCamActor->GetCameraComponent()->FieldOfView);
            }
            else
            {
                InOutPOV.FOV = FMath::Clamp(InOutPOV.FOV + ((AnimatedCamActor->GetCameraComponent()->FieldOfView - InitialFOV) * Scale), AnimatedCamActor->GetCameraComponent()->FieldOfView, InitialFOV);
            }
        }
        InOutPOV.FOV = FMath::Clamp<float>(InOutPOV.FOV, FOVMin, FOVMax);
    }

	return true;
}
