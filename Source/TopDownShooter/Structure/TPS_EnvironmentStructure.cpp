// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_EnvironmentStructure.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"

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

void ATPS_EnvironmentStructure::RemoveEffect(UTPS_StatsEffects* EffectToRemove)
{
	EffectToRemove->BeginDestroy();

	Effects.Remove(EffectToRemove);
}

void ATPS_EnvironmentStructure::AddEffect(UTPS_StatsEffects* EffectToAdd)
{
	Effects.Add(EffectToAdd);
}
