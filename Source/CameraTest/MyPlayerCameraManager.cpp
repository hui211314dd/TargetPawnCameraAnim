// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerCameraManager.h"
#include "MyCameraAnimInst.h"
#include "Engine/World.h"
#include "Camera/CameraModifier.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "Camera/CameraAnim.h"

class UMyCameraAnimInst* AMyPlayerCameraManager::PlayCustomCameraAnim(class UCameraAnim* Anim, float Rate /*= 1.f*/, float Scale /*= 1.f*/, float BlendInTime /*= 0.f*/, float BlendOutTime /*= 0.f*/, bool bLoop /*= false*/, bool bRandomStartTime /*= false*/, float Duration /*= 0.f*/, ECameraAnimPlaySpace::Type PlaySpace /*= ECameraAnimPlaySpace::CameraLocal*/, FRotator UserPlaySpaceRot /*= FRotator::ZeroRotator*/, bool bTargetPawn /*= false*/)
{
    // get a new instance and play it
    if (AnimCameraActor != NULL)
    {
        UMyCameraAnimInst* const Inst = Cast<UMyCameraAnimInst>(AllocCameraAnimInst());
        if (Inst)
        {
            if (Anim != nullptr && !Anim->bRelativeToInitialFOV)
            {
                Inst->InitialFOV = ViewTarget.POV.FOV;
            }
            Inst->LastCameraLoc = FVector::ZeroVector;		// clear LastCameraLoc
            Inst->Play(Anim, AnimCameraActor, Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration);
            Inst->SetPlaySpace(PlaySpace, UserPlaySpaceRot);
			Inst->SetIsTargetPawnSpace(bTargetPawn);
            return Inst;
        }
    }

    return NULL;
}

void AMyPlayerCameraManager::PostInitializeComponents()
{
	Super::Super::PostInitializeComponents();

	// Setup default camera modifiers
	if (DefaultModifiers.Num() > 0)
	{
		for (auto ModifierClass : DefaultModifiers)
		{
			// empty entries are not valid here, do work only for actual classes
			if (ModifierClass)
			{
				UCameraModifier* const NewMod = AddNewCameraModifier(ModifierClass);

				// cache ref to camera shake if this is it
				UCameraModifier_CameraShake* const ShakeMod = Cast<UCameraModifier_CameraShake>(NewMod);
				if (ShakeMod)
				{
					CachedCameraShakeMod = ShakeMod;
				}
			}
		}
	}

	// create CameraAnimInsts in pool
	for (int32 Idx = 0; Idx < MAX_ACTIVE_CAMERA_ANIMS; ++Idx)
	{
		AnimInstPool[Idx] = NewObject<UMyCameraAnimInst>(this);

		// add everything to the free list initially
		FreeAnims.Add(AnimInstPool[Idx]);
	}

	// spawn the temp CameraActor used for updating CameraAnims
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save these temp actors into a map
	AnimCameraActor = GetWorld()->SpawnActor<ACameraActor>(SpawnInfo);
}

void AMyPlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	ClearCachedPPBlends();

	// Loop through each camera modifier
	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ++ModifierIdx)
	{
		// Apply camera modification and output into DesiredCameraOffset/DesiredCameraRotation
		if ((ModifierList[ModifierIdx] != NULL) && !ModifierList[ModifierIdx]->IsDisabled())
		{
			// If ModifyCamera returns true, exit loop
			// Allows high priority things to dictate if they are
			// the last modifier to be applied
			if (ModifierList[ModifierIdx]->ModifyCamera(DeltaTime, InOutPOV))
			{
				break;
			}
		}
	}

	// Now apply CameraAnims
	// these essentially behave as the highest-pri modifier.
	for (int32 Idx = 0; Idx < ActiveAnims.Num(); ++Idx)
	{
		UCameraAnimInst* const AnimInst = ActiveAnims[Idx];

		if (AnimCameraActor && !AnimInst->bFinished)
		{
			// clear out animated camera actor
			InitTempCameraActor(AnimCameraActor, AnimInst);

			// evaluate the animation at the new time
			AnimInst->AdvanceAnim(DeltaTime, false);

			// Add weighted properties to the accumulator actor
			if (AnimInst->CurrentBlendWeight > 0.f)
			{
				UMyCameraAnimInst* MyAnimInst = Cast<UMyCameraAnimInst>(AnimInst);
				if (!MyAnimInst->PorcessTargetPawnCameraAnim(AnimCameraActor, AnimInst, InOutPOV))
				{
					ApplyAnimToCamera(AnimCameraActor, AnimInst, InOutPOV);
				}
			}
		}

		// changes to this are good for a single update, so reset this to 1.f after processing
		AnimInst->TransientScaleModifier = 1.f;

		// handle animations that have finished
		if (AnimInst->bFinished)
		{
			ReleaseCameraAnimInst(AnimInst);
			Idx--;		// we removed this from the ActiveAnims array
		}
	}

	// need to zero this when we are done with it.  playing another animation
	// will calc a new InitialTM for the move track instance based on these values.
	if (AnimCameraActor)
	{
		AnimCameraActor->TeleportTo(FVector::ZeroVector, FRotator::ZeroRotator);
	}
}
