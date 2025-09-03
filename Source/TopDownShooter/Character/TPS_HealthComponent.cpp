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
	return health;
}

void UTPS_HealthComponent::ReceveDamage(float damage)
{
	health -= damage;

	if (health <= 0.0f)
		DeadEvent_BP();

	OnHealthChange.Broadcast(health);
}

void UTPS_HealthComponent::DeadEvent_BP_Implementation()
{
	//BP
}

