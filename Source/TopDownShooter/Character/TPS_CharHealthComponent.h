// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPS_HealthComponent.h"
#include "TPS_CharHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChangeStrenght, float, ShieldStrenght, float, damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldRecovered);

UCLASS()
class TOPDOWNSHOOTER_API UTPS_CharHealthComponent : public UTPS_HealthComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Shield")
	FOnShieldChangeStrenght OnShieldChangeStrenght;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Shield")
	FOnShieldBroken OnShieldBroken;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Shield")
	FOnShieldRecovered OnShieldRecovered;

	FTimerHandle TimerHandle_CoolDownShieldTimer;
	FTimerHandle TimerHandle_ShieldRecoveryRateTimer;

protected:
	float ShieldStrenghtVar = 100.f;
	float MaxShieldStrenght = 100.f;

public:
	//variables for Shield Recoverying logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float CoolDownShieldIsBrokenRecoveryTime = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "Shield")
	float CoolDownShieldRecoveryTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float ShieldRecoveryValue = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheild")
	float ShieldRecoveryRate = 0.1f;

	//sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheildSounds")
	USoundBase* BreakShieldSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheildSounds")
	USoundBase* ShieldRecoveryingSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheildSounds")
	USoundBase* ShieldIsRecoveredSound = nullptr;

	void ChangeCurrentHealth(float ChangeValue) override;

	void ChangeShieldStrenght(float ChangeValue);

	UFUNCTION(BlueprintCallable)
	float GetShieldStrenght();

	void ShieldCoolDownEnd();

	void RecoveryShield();
};
