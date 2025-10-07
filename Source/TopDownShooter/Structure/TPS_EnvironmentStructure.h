// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
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

	bool AvialableForEffects_Implementation();
	bool AvialableForEffectsOnlyCPP() override;
};
