
#include "SWCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "SWWeapon.h"
#include "Engine/World.h"

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
	ZoomInterpSpeed = 15.f;
}

void ASWCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Weapon = GetWorld()->SpawnActor<ASWWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Weapon != nullptr)
	{
		Weapon->SetOwner(this);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
	}
}

void ASWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Zoom Interpolation
	float TargetFOV = bZoom ? ZoomedFOV : DefaultFOV;
	float CurrnetFov = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(CurrnetFov);
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
	PlayerInputComponent->BindAction("ToggleProne", IE_Pressed, this, &ASWCharacter::ToggleProne);

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

void ASWCharacter::ToggleProne()
{
	bProne = !bProne;

	ServerProne(bProne);
}

void ASWCharacter::DoZoomInterpotion()
{

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

void ASWCharacter::ServerProne_Implementation(bool isProne)
{
	bProne = isProne;

	OnRepProne();
}

bool ASWCharacter::ServerProne_Validate(bool isProne)
{
	return true;
}

void ASWCharacter::OnRepProne()
{
	if (bProne)
	{
		Controller->SetIgnoreMoveInput(true);
	}
}

void ASWCharacter::NotifyStandEnd()
{
	Controller->SetIgnoreMoveInput(false);
}

void ASWCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWCharacter, bEquipped);
	DOREPLIFETIME(ASWCharacter, bZoom);
	DOREPLIFETIME(ASWCharacter, bProne);
}

