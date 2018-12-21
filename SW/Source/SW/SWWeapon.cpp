// Fill out your copyright notice in the Description page of Project Settings.

#include "SWWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASWWeapon::ASWWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MuzzleSocketName = "MuzzleFlashSocket";
	fTimeBetweenShot = 0.1f;
	WeaponState = EWeaponState::EWeaponState_Idle;
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASWWeapon::BeginPlay()
{
	Super::BeginPlay();
}

// #Client - calc delay time, request Server Fire
void ASWWeapon::Fire()
{
	fLastFiredTime = GetWorld()->TimeSeconds;

	ServerFire(); // fire request
}

// #Client - Set FireTimer, request Server to change fire state.
void ASWWeapon::StartFire()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_FireDelay))
		return;

	// fTimeBetweenShot보다 input이 빠른 경우 고려
	float FirstDelayTime = FMath::Max(fLastFiredTime + fTimeBetweenShot - GetWorld()->TimeSeconds, 0.f);
	GetWorldTimerManager().SetTimer(TimerHandle_FireDelay, this, &ASWWeapon::Fire, fTimeBetweenShot, true, FirstDelayTime);

	ServerSetWeaponState(EWeaponState::EWeaponState_Fire);
}

// #Client - Clear FireTimer, request Server to change idle state.
void ASWWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_FireDelay);
	
	ServerSetWeaponState(EWeaponState::EWeaponState_Idle);
}

// #Server - increment shotcount, repnotify for all client.
void ASWWeapon::ServerFire_Implementation()
{
	++ShotCount;

	//@TODO: Trace Target & apply damage
}

bool ASWWeapon::ServerFire_Validate()
{
	return true;
}

// #Client - receive from server, play client effect.
void ASWWeapon::OnRep_ShotInfo()
{
	ClientPlayFireEffect();
}

void ASWWeapon::ClientPlayFireEffect_Implementation()
{
	if (MuzzleEffect != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}

bool ASWWeapon::ClientPlayFireEffect_Validate()
{
	return true;
}

void ASWWeapon::ServerSetWeaponState_Implementation(EWeaponState NewState)
{
	WeaponState = NewState;

	OnRep_WeaponStateChanged();
}

bool ASWWeapon::ServerSetWeaponState_Validate(EWeaponState NewState)
{
	return true;
}

void ASWWeapon::OnRep_WeaponStateChanged()
{
	switch (WeaponState)
	{
	case EWeaponState::EWeaponState_Idle:

		break;
	case EWeaponState::EWeaponState_Fire:

		break;
	case EWeaponState::EWeaponState_Reload:

		break;
	default:;
	}
}

void ASWWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWWeapon, WeaponState);
	DOREPLIFETIME(ASWWeapon, ShotCount);
}