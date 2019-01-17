// Fill out your copyright notice in the Description page of Project Settings.

#include "SWPlayerController.h"
#include "Components/InputComponent.h"
#include "SWCharacter.h"

void ASWPlayerController::BeginPlay()
{
	MyPawn = Cast<ASWCharacter>(GetPawn());
}