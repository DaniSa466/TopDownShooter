// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_CharHealthComponent.h"
#include "Kismet/GameplayStatics.h"

void UTPS_CharHealthComponent::ChangeCurrentHealth_OnServer(float ChangeValue)
{
	float DamageOnShield = ChangeValue * DamageCoef;

	if (ShieldStrenghtVar > 0.0f && ChangeValue < 0.f)
		ChangeShieldStrenght(DamageOnShield);
	
	else
		Super::ChangeCurrentHealth_OnServer(ChangeValue);
}

void UTPS_CharHealthComponent::ChangeShieldStrenght(float ChangeValue)
{
	ShieldStrenghtVar += ChangeValue;

	if (ShieldStrenghtVar > MaxShieldStrenght)
		ShieldStrenghtVar = MaxShieldStrenght;
	else
		if (ShieldStrenghtVar <= 0.0f)
			ShieldStrenghtVar = 0.0f;

	ShieldChangeStrenghtEvent_Multicast(ShieldStrenghtVar, ChangeValue);

	if (ShieldStrenghtVar == 0.0f)
	{
		ShieldBrokenEvent_Multicast();

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

		ShieldRecoveredEvent_Multicast();
	}
	else
		ShieldStrenghtVar = ShieldValueInNextStep;

	ShieldChangeStrenghtEvent_Multicast(ShieldStrenghtVar, ShieldRecoveryValue);
}

float UTPS_CharHealthComponent::GetMaxHealth()
{
	return maxHealth;
}

void UTPS_CharHealthComponent::IncreaseHealthByCoef(float coef)
{
	maxHealth *= coef;
	HealthValue *= coef;

	HealthIncreaseEffectEvent_Multicast(maxHealth, coef);
}

void UTPS_CharHealthComponent::DecreasehealthByCoef(float coef)
{
	maxHealth /= coef;
	HealthValue /= coef;

	HealthIncreaseEffectEvent_Multicast(maxHealth, coef);
}

void UTPS_CharHealthComponent::ShieldChangeStrenghtEvent_Multicast_Implementation(float shieldStrenght, float damage)
{
	OnShieldChangeStrenght.Broadcast(shieldStrenght, damage);
}

void UTPS_CharHealthComponent::HealthIncreaseEffectEvent_Multicast_Implementation(float maxHealthVal, float coef)
{
	OnHealthIncreaseEffect.Broadcast(maxHealthVal, coef);
}

void UTPS_CharHealthComponent::ShieldBrokenEvent_Multicast_Implementation()
{
	OnShieldBroken.Broadcast();
}

void UTPS_CharHealthComponent::ShieldRecoveredEvent_Multicast_Implementation()
{
	OnShieldRecovered.Broadcast();
}
