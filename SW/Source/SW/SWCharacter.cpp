
#include "SWCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ASWCharacter::ASWCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	DefaultFOV = CameraComp->FieldOfView;
	ZoomedFOV = 40.f;
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

	PlayerInputComponent->BindAction("DoJump", IE_Pressed, this, &ASWCharacter::DoJump);
	PlayerInputComponent->BindAction("EquipWeapon", IE_Pressed, this, &ASWCharacter::EquipWeapon);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASWCharacter::ZoomIn);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASWCharacter::ZoomOut);

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

void ASWCharacter::DoJump()
{
	Jump();
}

void ASWCharacter::EquipWeapon()
{
	ServerEquipped();
}

void ASWCharacter::ZoomIn()
{
	if (!bEquipped)
		return;

	ServerZoom(true);
}

void ASWCharacter::ZoomOut()
{

	ServerZoom(false);
}

void ASWCharacter::ServerEquipped_Implementation()
{
	bEquipped = !bEquipped;

	OnRepEquipped();
}

bool ASWCharacter::ServerEquipped_Validate()
{
	return true;
}

void ASWCharacter::ServerZoom_Implementation(bool isZoom)
{
	bZoom = isZoom;

	OnRepZoom();
}

bool ASWCharacter::ServerZoom_Validate(bool isZoom)
{
	return true;
}

void ASWCharacter::OnRepEquipped()
{
	if (bEquipped)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void ASWCharacter::OnRepZoom()
{
	if (bZoom)
	{
		CameraComp->SetFieldOfView(ZoomedFOV);
	}
	else
	{
		CameraComp->SetFieldOfView(DefaultFOV);
	}
}

void ASWCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWCharacter, bEquipped);
	DOREPLIFETIME(ASWCharacter, bZoom);
}

