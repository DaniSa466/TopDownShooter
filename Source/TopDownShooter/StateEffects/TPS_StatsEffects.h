// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TPS_StatsEffects.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWNSHOOTER_API UTPS_StatsEffects : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool InitObject(APawn* Player);
	virtual void ExecuteClass(float DeltaTime);
	virtual void DestroyObject();
};
