// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_StatsEffects.h"
#include "TopDownShooter/Character/TPS_HealthComponent.h"
#include "Kismet/GameplayStatics.h"

bool UTPS_StatsEffects::InitObject(AActor* ActorToInit)
{
	NewActor = ActorToInit;

	return true;
}

void UTPS_StatsEffects::DestroyObject()
{
	NewActor = nullptr;

	if (this && this->IsValidLowLevel())
	{
		this->BeginDestroy();
	}
}

bool UTPS_StatsEffects::ChackStackableEffect()
{
	return false;
}

bool UTPS_EffectExecuteOnce::InitObject(AActor* ActorToInit)
{
	Super::InitObject(ActorToInit);
}

void UTPS_EffectExecuteOnce::DestroyObject()
{
	Super::DestroyObject();
}

void UTPS_EffectExecuteOnce::ExecuteOnce()
{
	if (NewActor)
	{
		UTPS_HealthComponent* EffectPointerToHealthComponent = Cast<UTPS_HealthComponent>
			(NewActor->GetComponentByClass(UTPS_HealthComponent::StaticClass()));

		if (EffectPointerToHealthComponent)
			EffectPointerToHealthComponent->ChangeCurrentHealth(Power);
	}

	DestroyObject();
}

bool UTPS_EffectExecuteTimer::InitObject(AActor* ActorToInit)
{
	Super::InitObject(ActorToInit);

	GetWorld()->GetTimerManager().SetTimer(EffectTimer, this, 
		&UTPS_EffectExecuteTimer::DestroyObject, Timer, false);
	GetWorld()->GetTimerManager().SetTimer(ExecuteTimer, this,
		&UTPS_EffectExecuteTimer::Execute, RateTime, true);

	if (ParticleEffect)
	{
		FName BoneNameToAttachEffect;
		FVector Location;
		ParticleEmitter = UGameplayStatics::SpawnEmitterAttached(ParticleEffect,
			NewActor->GetRootComponent(), BoneNameToAttachEffect, Location,
			FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
	}

	return true;
}

void UTPS_EffectExecuteTimer::DestroyObject()
{
	ParticleEmitter->DestroyComponent();
	ParticleEmitter = nullptr;
	Super::DestroyObject();
}

void UTPS_EffectExecuteTimer::Execute()
{
	if (NewActor)
	{
		UTPS_HealthComponent* EffectPointerToHealthComponent = Cast<UTPS_HealthComponent>
			(NewActor->GetComponentByClass(UTPS_HealthComponent::StaticClass()));
		if (EffectPointerToHealthComponent)
			EffectPointerToHealthComponent->ChangeCurrentHealth(Power);
	}
}