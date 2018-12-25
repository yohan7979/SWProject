
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWWeapon;

UCLASS()
class SW_API ASWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASWCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	FVector GetSpringArmWorldPos() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Weapon")
	ASWWeapon* Weapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASWWeapon> WeaponClass;

private:
	void MoveForward(float fValue);
	void MoveRight(float fValue);
	void Turn(float fValue);
	void LookUp(float fValue);

	void BeginJump();
	void EndJump();
	void EquipWeapon();
	void ZoomIn();
	void ZoomOut();
	void ToggleProne();

	void StartFire();
	void StopFire();

public:
	UPROPERTY(ReplicatedUsing=OnRepEquipped, BlueprintReadOnly)
	bool bEquipped;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bZoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ZoomInterpSpeed;
	
	UPROPERTY(ReplicatedUsing=OnRepProne, BlueprintReadOnly)
	bool bProne;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimingPitch;

	float PrevPitch;
	float CurrentPitch;

public:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipped();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoom(bool isZoom);

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSetAimingPitch(float fPitch);

	UFUNCTION()
	void OnRepEquipped();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerProne(bool isProne);
	
	UFUNCTION()
	void OnRepProne();

	UFUNCTION(BlueprintCallable)
	void NotifyStandEnd();
};
