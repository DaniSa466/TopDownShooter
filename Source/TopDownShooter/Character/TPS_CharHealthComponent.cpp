// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_CharHealthComponent.h"
#include "Kismet/GameplayStatics.h"

void UTPS_CharHealthComponent::ChangeCurrentHealth(float ChangeValue)
{
	float DamageOnShield = ChangeValue * DamageCoef;

	if (ShieldStrenghtVar > 0.0f && ChangeValue < 0.f)
		ChangeShieldStrenght(DamageOnShield);
	
	else
		Super::ChangeCurrentHealth(ChangeValue);
}

void UTPS_CharHealthComponent::ChangeShieldStrenght(float ChangeValue)
{
	ShieldStrenghtVar += ChangeValue;

	if (ShieldStrenghtVar > MaxShieldStrenght)
		ShieldStrenghtVar = MaxShieldStrenght;
	else
		if (ShieldStrenghtVar <= 0.0f)
			ShieldStrenghtVar = 0.0f;

	OnShieldChangeStrenght.Broadcast(ShieldStrenghtVar, ChangeValue);

	if (ShieldStrenghtVar == 0.0f)
	{
		OnShieldBroken.Broadcast();

		UGameplayStatics::PlaySoundAtLocation(this, BreakShieldSound, 
			GetOwner()->GetActorLocation());

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_CoolDownShieldTimer,
				this, &UTPS_CharHealthComponent::ShieldCoolDownEnd,
				CoolDownShieldIsBrokenRecoveryTime, false);
		}
	}
	else
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_CoolDownShieldTimer,
				this, &UTPS_CharHealthComponent::ShieldCoolDownEnd,
				CoolDownShieldRecoveryTime, false);
		}
}

float UTPS_CharHealthComponent::GetShieldStrenght()
{
	return ShieldStrenghtVar;
}

void UTPS_CharHealthComponent::ShieldCoolDownEnd()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShieldRecoveryRateTimer,
			this, &UTPS_CharHealthComponent::RecoveryShield,
			ShieldRecoveryRate, true);

		UGameplayStatics::PlaySoundAtLocation(this, ShieldRecoveryingSound, 
			GetOwner()->GetActorLocation(), 3.5f);
	}
}

void UTPS_CharHealthComponent::RecoveryShield()
{
	float ShieldValueInNextStep = ShieldStrenghtVar + ShieldRecoveryValue;

	if (ShieldValueInNextStep > MaxShieldStrenght)
	{ 
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);

		UGameplayStatics::PlaySoundAtLocation(this, ShieldIsRecoveredSound, GetOwner()->GetActorLocation(), 2.f, 1.f, 0.7f);

		OnShieldRecovered.Broadcast();
	}
	else
		ShieldStrenghtVar = ShieldValueInNextStep;

	OnShieldChangeStrenght.Broadcast(ShieldStrenghtVar, ShieldRecoveryValue);
}

float UTPS_CharHealthComponent::GetMaxHealth()
{
	return maxHealth;
}

void UTPS_CharHealthComponent::IncreaseHealthByCoef(float coef)
{
	maxHealth *= coef;
	HealthValue *= coef;

	OnHealthIncreaseEffect.Broadcast(maxHealth, coef);
}

void UTPS_CharHealthComponent::DecreasehealthByCoef(float coef)
{
	maxHealth /= coef;
	HealthValue /= coef;

	OnHealthIncreaseEffect.Broadcast(maxHealth, coef);
}
