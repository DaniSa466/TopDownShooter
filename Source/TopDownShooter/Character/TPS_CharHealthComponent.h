// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPS_HealthComponent.h"
#include "TPS_CharHealthComponent.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWNSHOOTER_API UTPS_CharHealthComponent : public UTPS_HealthComponent
{
	GENERATED_BODY()
	
public:
	void ChangeCurrentHealth(float ChangeValue) override;
};
