// Fill out your copyright notice in the Description page of Project Settings.

#include "TPS_HealthComponent.h"
#include "Net/UnrealNetwork.h"

void UTPS_HealthComponent::RegenHealth()
{
	ChangeCurrentHealth_OnServer(3.f);
}

void UTPS_HealthComponent::StartRegen()
{
	GetWorld()->GetTimerManager().SetTimer(regen_TimerHandle, this, &UTPS_HealthComponent::RegenHealth, timeToRegen, true);
}

// Sets default values for this component's properties
UTPS_HealthComponent::UTPS_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// ...
}

// Called when the game starts
void UTPS_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UTPS_HealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UTPS_HealthComponent::GetResistToDamage()
{
	return resistToDamage;
}

void UTPS_HealthComponent::ChangeResistToDamage()
{
	resistToDamage = !resistToDamage;
}

float UTPS_HealthComponent::GetCurrentHealth()
{
	return HealthValue;
}

void UTPS_HealthComponent::SetCurrentHealth(float SetHealth)
{
	HealthValue = SetHealth;
}

void UTPS_HealthComponent::ChangeCurrentHealth_OnServer_Implementation(float ChangeValue)
{
	if (ChangeValue < 0)
	{
		if (GetWorld() && bCanRegen)
		{
			GetWorld()->GetTimerManager().ClearTimer(regen_TimerHandle);

			GetWorld()->GetTimerManager().SetTimer(startRegen_TimerHandle, this, 
				&UTPS_HealthComponent::StartRegen, timeToStartRegen, false);
		}

		if (resistToDamage)
			return;

		ChangeValue *= DamageCoef;
	}

	HealthValue += ChangeValue;
	HealthChangeEvent_Multicast(HealthValue, ChangeValue);

	if (HealthValue >= maxHealth)
	{
		HealthValue = maxHealth;

		if (GetWorld() && bCanRegen)
			GetWorld()->GetTimerManager().ClearTimer(regen_TimerHandle);
	}
	else
	{
		if (HealthValue <= 0.0f)
		{
			DeadEvent_Multicast();
		}
	}
}

void UTPS_HealthComponent::HealthChangeEvent_Multicast_Implementation(float healthVal, float changeValue)
{
	OnHealthChange.Broadcast(healthVal, changeValue);
}

void UTPS_HealthComponent::DeadEvent_Multicast_Implementation()
{
	OnDead.Broadcast();
}

void UTPS_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTPS_HealthComponent, HealthValue);
}