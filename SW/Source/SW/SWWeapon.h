// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;

UENUM()
enum class EWeaponState
{
	EWeaponState_Idle,
	EWeaponState_Fire,
	EWeaponState_Reload,
	EWeaponState_Max
};

UCLASS()
class SW_API ASWWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Fire();

public:	
	void StartFire();
	void StopFire();

	UFUNCTION(Client, Unreliable, WithValidation)
	void ClientPlayFireEffect();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetWeaponState(EWeaponState NewState);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Particles")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponStateChanged)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponStateChanged();

	UPROPERTY(ReplicatedUsing=OnRep_ShotInfo)
	int32 ShotCount;

	UFUNCTION()
	void OnRep_ShotInfo();

private:
	FTimerHandle TimerHandle_FireDelay;
	float fTimeBetweenShot;
	float fLastFiredTime;
	FName MuzzleSocketName;
};
