
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

private:
	void MoveForward(float fValue);
	void MoveRight(float fValue);
	void Turn(float fValue);
	void LookUp(float fValue);

	void DoJump();
	void EquipWeapon();
	void ZoomIn();
	void ZoomOut();

private:
	FTimerHandle TimerHandle_Zoom;
	void DoZoomInterpotion();

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

public:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipped();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerZoom(bool isZoom);

	UFUNCTION()
	void OnRepEquipped();
};
