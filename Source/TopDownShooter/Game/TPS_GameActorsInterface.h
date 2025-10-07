// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Event")
	bool AvialableForEffectsBP();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Event")
	bool AvialableForEffects();

	virtual bool AvialableForEffectsOnlyCPP();
};
