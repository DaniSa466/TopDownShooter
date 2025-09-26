// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPS_HealthComponent.h"
#include "TPS_CharHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChangeStrenght, float, ShieldStrenght, float, damage);

UCLASS()
class TOPDOWNSHOOTER_API UTPS_CharHealthComponent : public UTPS_HealthComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Shield")
	FOnShieldChangeStrenght OnShieldChangeStrenght;

	FTimerHandle TimerHandle_CoolDownShieldTimer;
	FTimerHandle TimerHandle_ShieldRecoveryRateTimer;

protected:
	float shield = 100.f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float CoolDownShieldRecoveryTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float ShieldRecoveryValue = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float ShieldRecoveryRate = 0.1f;

	void ChangeCurrentHealth(float ChangeValue) override;

	void ChangeShieldStrenght(float ChangeValue);

	UFUNCTION(BlueprintCallable)
	float GetShieldStrenght();

	void ShieldCoolDownEnd();

	void RecoveryShield();
};
