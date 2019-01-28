// Fill out your copyright notice in the Description page of Project Settings.

#include "SWWeapon.h"
#include "SWCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Actor.h"

// Sets default values
ASWWeapon::ASWWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MuzzleSocketName = "MuzzleFlashSocket";
	fTimeBetweenShot = 0.1f;
	DamageAmount = 20.f;
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

	// line trace in client side, request server to apply damage.
	FHitResult Hit;
	FVector TraceStart, TraceEnd, ShotDirection;

	ASWCharacter* Owner = Cast<ASWCharacter>(GetOwner());
	if (Owner)
	{
		// use controller rotation
		ShotDirection = Owner->GetControlRotation().Vector();
		ShotDirection.Normalize();

		// Start at spring arm position
		TraceStart = Owner->GetSpringArmWorldPos();
		TraceEnd = TraceStart + ShotDirection * 10000;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Owner);
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.bTraceComplex = true;

		// Do Line Trace
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_GameTraceChannel1, CollisionParams))
		{
			Request_ApplyDamage(Hit.GetActor(), DamageAmount, ShotDirection, Hit.BoneName);

			PlayImpactEffect(Hit.GetActor(), Hit.ImpactPoint);
			Server_PlayImpactEffect(Hit.GetActor(), Hit.ImpactPoint);
		}

		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, true, 3.f, 0, 1.f);
	}

	Server_Fire();

	// play owned client's weaponfx
	PlayFireEffect();
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

void ASWWeapon::StartReload()
{
	ServerSetWeaponState(EWeaponState::EWeaponState_Reload);
}

void ASWWeapon::EndReload()
{
	ServerSetWeaponState(EWeaponState::EWeaponState_Idle);
}

void ASWWeapon::PlayFireEffect()
{
	if (MuzzleEffect != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
}

void ASWWeapon::PlayImpactEffect(AActor* HitActor, const FVector& ImpactPoint)
{
	// 우선 Tag로 플레이어 구분하여 이펙트 선정
	UParticleSystem* SelectedEffect = nullptr;
	if (HitActor->ActorHasTag(TEXT("Player")))
	{
		SelectedEffect = FleshImpact;
	}
	else
	{
		SelectedEffect = DefaultImpact;
	}

	if (SelectedEffect != nullptr)
	{
		FVector ShotDirection = ImpactPoint - MeshComp->GetSocketLocation(MuzzleSocketName);
		ShotDirection.Normalize();

		FVector ImpactLocation = ImpactPoint + ShotDirection * -10;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactLocation, ShotDirection.Rotation());
	}
}

void ASWWeapon::Server_PlayImpactEffect_Implementation(AActor* HitActor, const FVector& ImpactPoint)
{
	Multicast_ImpactEffect(HitActor, ImpactPoint);
}

bool ASWWeapon::Server_PlayImpactEffect_Validate(AActor* HitActor, const FVector& ImpactPoint)
{
	return true;
}

void ASWWeapon::Multicast_ImpactEffect_Implementation(AActor* HitActor, const FVector& ImpactPoint)
{
	// Dedi server do not play weaponfx
	if (Role == ROLE_Authority)
	{
		return;
	}

	// play on SimluatedProxy, not owner client.
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && !Owner->IsLocallyControlled())
	{
		// play weaponfx for other client.
		PlayImpactEffect(HitActor, ImpactPoint);		
	}
}

bool ASWWeapon::Multicast_ImpactEffect_Validate(AActor* HitActor, const FVector& ImpactPoint)
{
	return true;
}

// #Server
void ASWWeapon::Server_Fire_Implementation()
{
	// increment shotcount, repnotify for all client.
	++ShotCount;

	/* FX
		네트워크 지연시간을 고려하여 주체자 클라에서는 즉각적으로 처리하고,
		서버가 멀티캐스트를 호출해서 타 클라와 동기화되도록 처리한다. */
	Multicast_FireEffect();
}

bool ASWWeapon::Server_Fire_Validate()
{
	return true;
}

// #Server - ApplyPointDamage
void ASWWeapon::Request_ApplyDamage_Implementation(AActor* DamagedActor, float BaseDamage, const FVector& HitFromDirection, FName BoneName)
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FHitResult Hit(1.f);
		Hit.BoneName = BoneName;

		UGameplayStatics::ApplyPointDamage(DamagedActor, BaseDamage, HitFromDirection, Hit, Owner->GetInstigatorController(), Owner, nullptr);
	}

}

bool ASWWeapon::Request_ApplyDamage_Validate(AActor* DamagedActor, float BaseDamage, const FVector& HitFromDirection, FName BoneName)
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
		
	// play on SimluatedProxy, not owner client.
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner && !Owner->IsLocallyControlled())
	{
		// play weaponfx for other client.
		PlayFireEffect();	
	}
}

bool ASWWeapon::Multicast_FireEffect_Validate()
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