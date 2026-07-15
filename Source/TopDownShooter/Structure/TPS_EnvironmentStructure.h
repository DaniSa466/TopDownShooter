// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "TPS_EnvironmentStructure.generated.h"

UCLASS()
class TOPDOWNSHOOTER_API ATPS_EnvironmentStructure : public AActor, public ITPS_GameActorsInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPS_EnvironmentStructure();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	EPhysicalSurface GetSurfaceType() override;

	TArray<UTPS_StatsEffects*> GetCurrentEffects() override;

	void RemoveEffect(UTPS_StatsEffects* effectToRemove) override;
	void AddEffect(UTPS_StatsEffects* effectToAdd) override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<UTPS_StatsEffects*> Effects;
};
