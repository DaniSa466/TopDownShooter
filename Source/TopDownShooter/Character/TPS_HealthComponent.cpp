// Fill out your copyright notice in the Description page of Project Settings.

#include "TPS_HealthComponent.h"

// Sets default values for this component's properties
UTPS_HealthComponent::UTPS_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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

float UTPS_HealthComponent::GetCurrentHealth()
{
	return HealthValue;
}

void UTPS_HealthComponent::SetCurrentHealth(float SetHealth)
{
	HealthValue = SetHealth;
}

void UTPS_HealthComponent::ChangeCurrentHealth(float ChangeValue)
{
	ChangeValue *= DamageCoef;

	HealthValue += ChangeValue;
	OnHealthChange.Broadcast(HealthValue, ChangeValue);

	if (HealthValue > 100.f)
		HealthValue = 100.f;
	else
	{

		if (HealthValue <= 0.0f)
		{
			OnDead.Broadcast();
		}
	}
}
