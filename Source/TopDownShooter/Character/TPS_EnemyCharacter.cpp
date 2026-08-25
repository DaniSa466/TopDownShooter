// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_EnemyCharacter.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "Engine/ActorChannel.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"

ATPS_EnemyCharacter::ATPS_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ATPS_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ATPS_EnemyCharacter::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool wrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (int32 i = 0; i < Effects.Num(); i++)
	{
		if (Effects[i])
			wrote |= Channel->ReplicateSubobject(Effects[i], *Bunch, *RepFlags);
	}

	return wrote;
}

void ATPS_EnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATPS_EnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATPS_EnemyCharacter::RemoveEffect(UTPS_StatsEffects* effectToRemove)
{
	Effects.Remove(effectToRemove);

	SwitchEffect(effectToRemove, false);
	effectRemove = effectToRemove;
}

void ATPS_EnemyCharacter::AddEffect(UTPS_StatsEffects* effectToAdd)
{
	Effects.Add(effectToAdd);

	if (!effectToAdd->isAutoDestroy)
	{
		SwitchEffect(effectToAdd, true);
		effectAdd = effectToAdd;
	}
	else
	{
		if (effectToAdd->ParticleEffect)
			ExecuteEffectAdd_OnServer(effectToAdd->ParticleEffect);
	}
}

void ATPS_EnemyCharacter::OnRep_EffectAdd()
{
	if (effectAdd)
		SwitchEffect(effectAdd, true);
}

void ATPS_EnemyCharacter::OnRep_EffectRemove()
{
	if (effectRemove)
		SwitchEffect(effectRemove, false);
}

void ATPS_EnemyCharacter::ExecuteEffectAdd_OnServer_Implementation(UParticleSystem* effectFX)
{
	ExecuteEffectAdd_Multicast(effectFX);
}

void ATPS_EnemyCharacter::ExecuteEffectAdd_Multicast_Implementation(UParticleSystem* effectFX)
{
	UTypes::ExecuteEffectAdded(effectFX, this, FVector(0), FName("Spine_01"));
}

void ATPS_EnemyCharacter::SwitchEffect(UTPS_StatsEffects* effect, bool bIsAdd)
{
	if (bIsAdd)
	{
		if (effect && effect->ParticleEffect)
		{
			FName nameBoneToAttach = effect->boneName;
			FVector location = FVector(0);

			USkeletalMeshComponent* mySkelMesh = GetMesh();
			if (mySkelMesh)
			{
				UParticleSystemComponent* newParticleSystem = UGameplayStatics::SpawnEmitterAttached(
					effect->ParticleEffect, mySkelMesh, nameBoneToAttach, location,
					FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);

				particleSystemEffects.Add(newParticleSystem);
			}
		}
	}
	else
	{
		int32 i = 0;
		bool bIsFound = false;

		while (i < particleSystemEffects.Num() && !bIsFound)
		{
			if (particleSystemEffects[i] && effect->ParticleEffect && particleSystemEffects[i]->Template
				&& effect->ParticleEffect == particleSystemEffects[i]->Template)
			{
				bIsFound = true;
				particleSystemEffects[i]->DeactivateSystem();
				//newEffect->DestroyObject();
				//particleSystemEffects[i]->DestroyComponent();
				particleSystemEffects.RemoveAt(i);
			}

			i++;
		}
	}
}

void ATPS_EnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPS_EnemyCharacter, Effects);
	DOREPLIFETIME(ATPS_EnemyCharacter, effectAdd);
	DOREPLIFETIME(ATPS_EnemyCharacter, effectRemove);
}