// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_EnvironmentStructure.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/ActorChannel.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATPS_EnvironmentStructure::ATPS_EnvironmentStructure()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ATPS_EnvironmentStructure::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ATPS_EnvironmentStructure::ReplicateSubobjects(UActorChannel* Channel, 
	FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool wrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (int32 i = 0; i < Effects.Num(); i++)
	{
		if (Effects[i])
			wrote |= Channel->ReplicateSubobject(Effects[i], *Bunch, *RepFlags);
	}

	return wrote;
}

// Called every frame
void ATPS_EnvironmentStructure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

EPhysicalSurface ATPS_EnvironmentStructure::GetSurfaceType()
{
	EPhysicalSurface result = EPhysicalSurface::SurfaceType_Default;
	UStaticMeshComponent* myMesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (myMesh)
	{
		UMaterialInterface* myMaterial = myMesh->GetMaterial(0);

		if (myMaterial)
		{
			result = myMaterial->GetPhysicalMaterial()->SurfaceType;
		}
	}

	return result;
}

TArray<UTPS_StatsEffects*> ATPS_EnvironmentStructure::GetCurrentEffects()
{
	return Effects;
}

void ATPS_EnvironmentStructure::RemoveEffect(UTPS_StatsEffects* effectToRemove)
{
	//effectToRemove->BeginDestroy();

	Effects.Remove(effectToRemove);

	SwitchEffect(effectToRemove, false);
	effectRemove = effectToRemove;
}

void ATPS_EnvironmentStructure::AddEffect(UTPS_StatsEffects* effectToAdd)
{
	Effects.Add(effectToAdd);

	if (!effectToAdd->isAutoDestroy)
	{
		SwitchEffect(effectToAdd, true);
		effectAdd = effectToAdd;
	}
	else
		if (effectToAdd->ParticleEffect)
			ExecuteEffectAdd_OnServer(effectToAdd->ParticleEffect);
}

void ATPS_EnvironmentStructure::OnRep_EffectAdd()
{
	if (effectAdd)
		SwitchEffect(effectAdd, true);
}

void ATPS_EnvironmentStructure::OnRep_EffectRemove()
{
	if (effectRemove)
		SwitchEffect(effectRemove, false);
}

void ATPS_EnvironmentStructure::ExecuteEffectAdd_OnServer_Implementation(UParticleSystem* effectFX)
{
	ExecuteEffectAdd_Multicast(effectFX);
}

void ATPS_EnvironmentStructure::ExecuteEffectAdd_Multicast_Implementation(UParticleSystem* effectFX)
{
	UTypes::ExecuteEffectAdded(effectFX, this, effectOffset, NAME_None);
}

void ATPS_EnvironmentStructure::SwitchEffect(UTPS_StatsEffects* effect, bool bIsAdd)
{
	if (bIsAdd)
	{
		if (effect && effect->ParticleEffect)
		{
			FName nameBoneToAttach = NAME_None;
			FVector location = effectOffset;

			USceneComponent* mySceneComp = GetRootComponent();
			if (mySceneComp)
			{
				UParticleSystemComponent* newParticleSystem = UGameplayStatics::SpawnEmitterAttached(
					effect->ParticleEffect, mySceneComp, nameBoneToAttach, location,
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

void ATPS_EnvironmentStructure::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPS_EnvironmentStructure, Effects);
	DOREPLIFETIME(ATPS_EnvironmentStructure, effectAdd);
	DOREPLIFETIME(ATPS_EnvironmentStructure, effectRemove);
}
