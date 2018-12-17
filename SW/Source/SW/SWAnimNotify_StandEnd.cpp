// Fill out your copyright notice in the Description page of Project Settings.

#include "SWAnimNotify_StandEnd.h"
#include "Components/ActorComponent.h"
#include "SWCharacter.h"

void USWAnimNotify_StandEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != nullptr)
	{
		AActor* Owner = MeshComp->GetOwner();
		if (Owner != nullptr)
		{
			ASWCharacter* MyCharacter = Cast<ASWCharacter>(Owner);
			if (MyCharacter != nullptr)
			{
				MyCharacter->NotifyStandEnd();
			}
		}
	}
}
