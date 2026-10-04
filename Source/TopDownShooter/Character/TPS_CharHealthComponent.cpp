// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_CharHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TopDownShooter/Character/TopDownShooterCharacter.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "TopDownShooter/FuncLibrary/Types.h"

void UTPS_CharHealthComponent::ChangeCurrentHealth_OnServer(float ChangeValue)
{
	float DamageOnShield = ChangeValue * DamageCoef;

	ATopDownShooterCharacter* character = nullptr;

	if (ChangeValue < 0.f)
	{
		bool isStun = FMath::FRand() < stunChance;

		if (isStun)
		{
			character = Cast<ATopDownShooterCharacter>(GetOwner());
			if (character && stunEffect)
			{
				UTypes::AddEffectBySurfaceType(character, NAME_None, stunEffect, EPhysicalSurface::SurfaceType3);
			}
		}
	}

	if (ShieldStrenghtVar > 0.0f && ChangeValue < 0.f)
	{
		ChangeShieldStrenght(DamageOnShield);
	}
	
	else
	{
		Super::ChangeCurrentHealth_OnServer(ChangeValue);

		if (ChangeValue < 0.0f)
		{
			uint8 hitReactionSoundIndex = FMath::RandHelper(hitReactionSounds.Num());
			if (FMath::RandBool())
			{
				/*
				first checking to valid of character reference in this code block is done for some kind of optimization.
				it's smth like "if chasracter was stunned, so this reference is valid and programm doesn't have to
				make one more cast. but probability that this reference is invelid is much bigger because of stun chance,
				so i have to add one more cast in that case. why i didn't just make a cast in the begining of this function?
				because there are a 0.9 * 0.5 chance that cast is unnececery ( if character is not stunned by hit and doesn't
				play hit reaction sound). so in 45% of hit to character this method doesn't need this cast.
				and the second checking to valid is a main logic. if that ref is ok, so play sound.
				*/

				if (!character)
					character = Cast<ATopDownShooterCharacter>(GetOwner());

				if (character)
					character->PlaySoundAtached_Multicast(hitReactionSounds[hitReactionSoundIndex]);
			}
		}
	}
}

void UTPS_CharHealthComponent::ChangeShieldStrenght(float ChangeValue)
{
	ShieldStrenghtVar += ChangeValue;

	if (ShieldStrenghtVar > MaxShieldStrenght)
	{
		ShieldStrenghtVar = MaxShieldStrenght;
	}
	else
	{
		if (ShieldStrenghtVar <= 0.0f)
		{
			ShieldStrenghtVar = 0.0f;
		}
	}

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
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ShieldRecoveryRateTimer);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_CoolDownShieldTimer,
				this, &UTPS_CharHealthComponent::ShieldCoolDownEnd,
				CoolDownShieldRecoveryTime, false);
		}
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
	{
		ShieldStrenghtVar = ShieldValueInNextStep;
	}

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
