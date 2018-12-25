// Fill out your copyright notice in the Description page of Project Settings.

#include "SWWeapon.h"
#include "SWCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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

	// @TODO
	// 네트워크 지연시간을 고려하여 주체자 클라에서는 즉각적으로 처리하고,
	// 서버가 멀티캐스트를 호출해서 타 클라와 동기화되도록 처리한다.

	// line trace in client, send hit result to server
	FHitResult Hit;
	FVector TraceStart, TraceEnd;

	ASWCharacter* MyOwner = Cast<ASWCharacter>(GetOwner());
	if (MyOwner)
	{
		// Start at spring arm position
		TraceStart = MyOwner->GetSpringArmWorldPos();

		// use actor rotation
		//TraceEnd = TraceStart + MyOwner->GetActorRotation().Vector() * 1000;
		TraceEnd = TraceStart + MyOwner->GetControlRotation().Vector() * 1000;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(MyOwner);
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.bTraceComplex = true;
		//CollisionParams.bReturnPhysicalMaterial = true;

		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_GameTraceChannel1, CollisionParams))
		{
			// request server to apply damage with hit info
			ServerFire();
		}

		// play owned client's weaponfx
		PlayFireEffect();

		// request server to multicast weaponfx(exclude owner client)
		Server_PlayFireEffect();

		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, true, 3.f, 0, 1.f);
	}
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

void ASWWeapon::PlayFireEffect()
{
	if (MuzzleEffect != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}

void ASWWeapon::Server_PlayFireEffect_Implementation()
{
	Multicast_FireEffect();
}

bool ASWWeapon::Server_PlayFireEffect_Validate()
{
	return true;
}

void ASWWeapon::Multicast_FireEffect_Implementation()
{
	// Dedi server do not play weaponfx
	if (Role == ROLE_Authority)
	{
		return;
	}
		
	// Check controller, if they are same, don't play cause it was played in owner client.
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		AController* OwnerPC = Owner->GetController();
		AController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (OwnerPC != LocalPC)
		{
			// play weaponfx for other client.
			PlayFireEffect();
		}
	}
}

bool ASWWeapon::Multicast_FireEffect_Validate()
{
	return true;
}

// #Server - increment shotcount, repnotify for all client.
void ASWWeapon::ServerFire_Implementation()
{
	++ShotCount;

	Multicast_FireEffect();

	//@TODO: Trace Target & apply damage
}

bool ASWWeapon::ServerFire_Validate()
{
	return true;
}

// #Client - receive from server, play client effect.
void ASWWeapon::OnRep_ShotInfo()
{

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