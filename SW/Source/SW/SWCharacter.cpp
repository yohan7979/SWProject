
#include "SWCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASWCharacter::ASWCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);


}

void ASWCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASWCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASWCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASWCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASWCharacter::LookUp);

}

void ASWCharacter::MoveForward(float fValue)
{
	float roll, pitch, yaw;
	
	UKismetMathLibrary::BreakRotator(GetControlRotation(), roll, pitch, yaw);
	FRotator rot = UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw);
	FVector vForward = UKismetMathLibrary::GetForwardVector(rot);

	AddMovementInput(vForward, fValue);
}

void ASWCharacter::MoveRight(float fValue)
{
	float roll, pitch, yaw;

	UKismetMathLibrary::BreakRotator(GetControlRotation(), roll, pitch, yaw);
	FRotator rot = UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw);
	FVector vRight = UKismetMathLibrary::GetRightVector(rot);

	AddMovementInput(vRight, fValue);
}

void ASWCharacter::Turn(float fValue)
{
	AddControllerYawInput(fValue);
}

void ASWCharacter::LookUp(float fValue)
{
	AddControllerPitchInput(fValue);
}

