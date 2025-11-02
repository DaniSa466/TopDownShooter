// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "TPS_GameActorsInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTPS_GameActorsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPDOWNSHOOTER_API ITPS_GameActorsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual EPhysicalSurface GetSurfaceType();

	virtual TArray<UTPS_StatsEffects*> GetCurrentEffects();

	virtual void RemoveEffect(UTPS_StatsEffects* EffectToRemove);
	virtual void AddEffect(UTPS_StatsEffects* EffectToAdd);
};
