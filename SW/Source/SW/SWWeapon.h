// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class UAnimMontage;

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

	void StartReload();
	void EndReload();

	void PlayFireEffect();
	void PlayImpactEffect(AActor* HitActor, const FVector& ImpactPoint);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetWeaponState(EWeaponState NewState);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire();

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_PlayImpactEffect(AActor* HitActor, const FVector& ImpactPoint);

	UFUNCTION(Server, Reliable, WithValidation)
	void Request_ApplyDamage(AActor* DamagedActor, float BaseDamage, const FVector& HitFromDirection, FName BoneName);

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void Multicast_FireEffect();

	UFUNCTION(NetMulticast, Unreliable, WithValidation)
	void Multicast_ImpactEffect(AActor* HitActor, const FVector& ImpactPoint);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	UParticleSystem* FleshImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	UParticleSystem* DefaultImpact;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponStateChanged)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponStateChanged();

	UPROPERTY(ReplicatedUsing=OnRep_ShotInfo)
	int32 ShotCount;

	UFUNCTION()
	void OnRep_ShotInfo();

	float DamageAmount;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* ReloadAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* EquipAnim;

private:
	FTimerHandle TimerHandle_FireDelay;
	float fTimeBetweenShot;
	float fLastFiredTime;
	FName MuzzleSocketName;
};
