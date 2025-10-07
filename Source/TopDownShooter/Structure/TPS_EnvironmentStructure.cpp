// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_EnvironmentStructure.h"

// Sets default values
ATPS_EnvironmentStructure::ATPS_EnvironmentStructure()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATPS_EnvironmentStructure::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATPS_EnvironmentStructure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATPS_EnvironmentStructure::AvialableForEffects_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ATPS_EnvironmentStructure::AvialableForEffects_Implementation"));
	return true;
}

bool ATPS_EnvironmentStructure::AvialableForEffectsOnlyCPP()
{
	UE_LOG(LogTemp, Warning, TEXT("ATPS_EnvironmentStructure::AvialableForEffectsOnlyCPP"));
	return false;
}
