// Fill out your copyright notice in the Description page of Project Settings.

#include "TPS_StatsEffects.h"
#include "TopDownShooter/Character/TPS_HealthComponent.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "TopDownShooter/Character/TopDownShooterCharacter.h"
#include "TopDownShooter/Game/TopDownShooterPlayerController.h"
#include "TopDownShooter/Character/TPS_CharHealthComponent.h"
#include "Kismet/GameplayStatics.h"

bool UTPS_StatsEffects::InitObject(AActor* ActorToInit, FName hitBoneName)
{
	newActor = ActorToInit;
	boneName = hitBoneName;

	ITPS_GameActorsInterface* myInterface = Cast<ITPS_GameActorsInterface>(newActor);
	if (myInterface)
		myInterface->AddEffect(this);

	return true;
}

void UTPS_StatsEffects::DestroyObject()
{
	ITPS_GameActorsInterface* myInterface = Cast<ITPS_GameActorsInterface>(newActor);
	if (myInterface)
		myInterface->RemoveEffect(this);

	newActor = nullptr;

	if (this && this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

bool UTPS_StatsEffects::ChackStackableEffect()
{
	return false;
}

bool UTPS_EffectExecuteOnce::InitObject(AActor* ActorToInit, FName hitBoneName)
{
	Super::InitObject(ActorToInit, hitBoneName);
	ExecuteOnce();

	return true;
}

void UTPS_EffectExecuteOnce::DestroyObject()
{
	Super::DestroyObject();
}

void UTPS_EffectExecuteOnce::ExecuteOnce()
{
	if (newActor)
	{
		UTPS_HealthComponent* EffectPointerToHealthComponent = Cast<UTPS_HealthComponent>
			(newActor->GetComponentByClass(UTPS_HealthComponent::StaticClass()));

		if (EffectPointerToHealthComponent)
			EffectPointerToHealthComponent->ChangeCurrentHealth_OnServer(Power);
	}

	DestroyObject();
}

bool UTPS_TemporaryEffect::InitObject(AActor* ActorToInit, FName hitBoneName)
{
	Super::InitObject(ActorToInit, hitBoneName);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(EffectTimer, this,
			&UTPS_TemporaryEffect::DestroyObject, Timer, false);
		GetWorld()->GetTimerManager().SetTimer(ExecuteTimer, this,
			&UTPS_TemporaryEffect::Execute, RateTime, true);
	}

	if (ParticleEffect)
	{
		FName BoneNameToAttachEffect = hitBoneName;
		FVector Location;
		USceneComponent* myMesh = Cast<USceneComponent>(newActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (myMesh)
		{
			ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
				myMesh, BoneNameToAttachEffect, Location,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		}
		else
		{
			ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
				newActor->GetRootComponent(), BoneNameToAttachEffect, Location,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		}
	}

	return true;
}

void UTPS_TemporaryEffect::DestroyObject()
{
	if (GetWorld())
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (ParticleEmitter == nullptr)
		return Super::DestroyObject();

	//ParticleEmitter->DestroyComponent();
	//ParticleEmitter = nullptr;
	Super::DestroyObject();
}

void UTPS_TemporaryEffect::Execute()
{
	if (newActor)
	{
		UTPS_HealthComponent* EffectPointerToHealthComponent = Cast<UTPS_HealthComponent>
			(newActor->GetComponentByClass(UTPS_HealthComponent::StaticClass()));
		if (EffectPointerToHealthComponent)
			EffectPointerToHealthComponent->ChangeCurrentHealth_OnServer(Power);
	}
}

bool UTPS_SpeedUpEffect::InitObject(AActor* ActorToSpeedUp, FName hitBoneName)
{
	Super::InitObject(ActorToSpeedUp, hitBoneName);
	pointerToCharacter = Cast<ATopDownShooterCharacter>(ActorToSpeedUp);

	if (!pointerToCharacter)
		return false;

	IncreaseSpeed();

	if (GetWorld())
		GetWorld()->GetTimerManager().SetTimer(decreaseTimer, this,
			&UTPS_SpeedUpEffect::DestroyObject, speedUpTimer, false);

	return true;
}

void UTPS_SpeedUpEffect::DestroyObject()
{
	pointerToCharacter->SetSpeedCoef();
	pointerToCharacter->OnDisableSpeedUpEffect.Broadcast();

	Super::DestroyObject();
}

void UTPS_SpeedUpEffect::IncreaseSpeed()
{
	pointerToCharacter->SetSpeedCoef(speedUpCoef);
	pointerToCharacter->OnEnableSpeedUpEffect.Broadcast();
}

bool UTPS_EffectsToHealth::InitObject(AActor* actorToInit, FName hitBoneName)
{
	Super::InitObject(actorToInit, hitBoneName);
	pointerToCharacter = Cast<ATopDownShooterCharacter>(actorToInit);

	if (!pointerToCharacter)
		return false;

	pointerToHealthComponent = pointerToCharacter->HealthComponent;
	if (!pointerToHealthComponent)
		return false;

	ChangeHealthCoef();

	if(GetWorld())
		GetWorld()->GetTimerManager().SetTimer(backTimer, this, 
			&UTPS_EffectsToHealth::DestroyObject, timer, false);

	return true;
}

void UTPS_EffectsToHealth::DestroyObject()
{
	//resist to damage effect
	if (healthCoef == 0)
	{
		pointerToCharacter->HealthComponent->ChangeResistToDamage();

		if (ParticleEmitter)
		{
			//ParticleEmitter->DestroyComponent();
			//ParticleEmitter = nullptr;
		}
	}
	//increasing health effect
	else
	{
		pointerToHealthComponent->DecreasehealthByCoef(healthCoef);
	}

	Super::DestroyObject();
}

void UTPS_EffectsToHealth::ChangeHealthCoef()
{
	//resist to damage effect
	if (healthCoef == 0)
	{
		pointerToCharacter->HealthComponent->ChangeResistToDamage();

		USkeletalMeshComponent* characterMesh = pointerToCharacter->GetMesh();
		if (ParticleEffect && characterMesh)
		{
			FName BoneNameToAttachEffect = "ik_foot_root";

			if (!characterMesh->DoesSocketExist(BoneNameToAttachEffect))
			{
				UE_LOG(LogTemp, Warning, TEXT("UTPS_EffectsToHealth::ChangeHealthCoef - Bone not found, attaching to root component"));
				ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
					characterMesh, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget, false);
			}
			else
				ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
				characterMesh, BoneNameToAttachEffect, FVector::ZeroVector,
				FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		}
	}
	//increasing health effect
	else
	{
		pointerToHealthComponent->IncreaseHealthByCoef(healthCoef);
	}
}

bool UTPS_StunEffect::InitObject(AActor* ActorToStun, FName hitBoneName)
{
	Super::InitObject(ActorToStun, hitBoneName);
	pointerToCharacter = Cast<ATopDownShooterCharacter>(ActorToStun);

	if (!pointerToCharacter)
		return false;

	ChangeCharacterInputStatus(true);

	if (GetWorld())
		GetWorld()->GetTimerManager().SetTimer(stunTimer, this, 
			&UTPS_StunEffect::DestroyObject, timer, false);

	return true;
}

void UTPS_StunEffect::DestroyObject()
{
	ChangeCharacterInputStatus(false);
	
	if (ParticleEmitter)
	{
		//ParticleEmitter->DestroyComponent();
		//ParticleEmitter = nullptr;
	}

	Super::DestroyObject();
}

void UTPS_StunEffect::ChangeCharacterInputStatus(bool isStun)
{
	if (isStun)
	{
		if (loopAnimation)
			pointerToCharacter->PlayAnimMontage(loopAnimation);

		pointerToCharacter->ResSpeed = 0;
		pointerToCharacter->DisableInput(Cast<APlayerController>(pointerToCharacter->GetController()));

		USkeletalMeshComponent* characterMesh = pointerToCharacter->GetMesh();
		if (ParticleEffect && characterMesh)
		{
			FName BoneNameToAttachEffect = "head";

			if (!characterMesh->DoesSocketExist(BoneNameToAttachEffect))
			{
				UE_LOG(LogTemp, Warning, TEXT("UTPS_EffectsToHealth::ChangeHealthCoef - Bone not found, attaching to root component"));
				ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
					characterMesh, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget, false);
			}
			else
				ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
					characterMesh, BoneNameToAttachEffect, FVector::ZeroVector,
					FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		}
	}
	else
	{
		pointerToCharacter->EnableInput(Cast<APlayerController>(pointerToCharacter->GetController()));
		pointerToCharacter->ChangeMovementState();
	}
}
