// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_CharHealthComponent.h"

void UTPS_CharHealthComponent::ChangeCurrentHealth(float ChangeValue)
{
	float DamageOnShield = ChangeValue * DamageCoef;

	if (shield > 0.0f && ChangeValue < 0.f)
	{
		ChangeShieldStrenght(DamageOnShield);
		
		if (shield <= 0.0f)
			UE_LOG(LogTemp, Warning, TEXT("TPS_CharHealthComponent::ChangeCurrentHealth - shield is broken"))
	}

	else
		Super::ChangeCurrentHealth(ChangeValue);
}

void UTPS_CharHealthComponent::ChangeShieldStrenght(float ChangeValue)
{
	shield += ChangeValue;

	if (shield > 100.f)
		shield = 100.f;
	else
		if (shield <= 0.0f)
			shield = 0.0f;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CoolDownShieldTimer,
			this, &UTPS_CharHealthComponent::ShieldCoolDownEnd, 
			CoolDownShieldRecoveryTime, false);

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);
	}

	OnShieldChangeStrenght.Broadcast(shield, ChangeValue);
}

float UTPS_CharHealthComponent::GetShieldStrenght()
{
	return shield;
}

void UTPS_CharHealthComponent::ShieldCoolDownEnd()
{
	if (GetWorld())
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShieldRecoveryRateTimer,
			this, &UTPS_CharHealthComponent::RecoveryShield, 
			ShieldRecoveryRate, true);
}

void UTPS_CharHealthComponent::RecoveryShield()
{
	float ShieldValueInNextStep = shield + ShieldRecoveryValue;

	if (ShieldValueInNextStep > 100.f)
	{ 
		shield = 100.f;
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);
	}
	else
		shield = ShieldValueInNextStep;

	OnShieldChangeStrenght.Broadcast(shield, ShieldRecoveryValue);
}
