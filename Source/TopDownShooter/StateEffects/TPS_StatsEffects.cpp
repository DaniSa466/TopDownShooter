// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "TPS_StatsEffects.h"
#include "TopDownShooter/Character/TPS_HealthComponent.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "Kismet/GameplayStatics.h"

bool UTPS_StatsEffects::InitObject(AActor* ActorToInit)
{
	NewActor = ActorToInit;

	ITPS_GameActorsInterface* myInterface = Cast<ITPS_GameActorsInterface>(NewActor);
	if (myInterface)
		myInterface->AddEffect(this);

	return true;
}

void UTPS_StatsEffects::DestroyObject()
{
	ITPS_GameActorsInterface* myInterface = Cast<ITPS_GameActorsInterface>(NewActor);
	if (myInterface)
		myInterface->RemoveEffect(this);

	NewActor = nullptr;

	if (this && this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

bool UTPS_StatsEffects::ChackStackableEffect()
{
	return false;
}

bool UTPS_EffectExecuteOnce::InitObject(AActor* ActorToInit)
{
	Super::InitObject(ActorToInit);
	ExecuteOnce();

	return true;
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