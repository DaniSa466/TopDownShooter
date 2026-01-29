// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TopDownShooter/Character/TopDownShooterCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "TPS_StatsEffects.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_StatsEffects : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool InitObject(AActor* ActorToInit);
	virtual void DestroyObject();

	virtual bool ChackStackableEffect();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	TArray <TEnumAsByte<EPhysicalSurface>> PossibleInteractSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool bIsStackable = false;

	AActor* newActor = nullptr;
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_EffectExecuteOnce : public UTPS_StatsEffects
{
	GENERATED_BODY()

public:
	bool InitObject(AActor* ActorToInit) override;
	void DestroyObject() override;
	void ExecuteOnce();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Once Setting")
	float Power = 20.f;
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_TemporaryEffect : public UTPS_StatsEffects
{
	GENERATED_BODY()

public:
	bool InitObject(AActor* ActorToInit) override;
	void DestroyObject() override;
	void Execute();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float Power = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float Timer = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float RateTime = 1.f;

	FTimerHandle ExecuteTimer;
	FTimerHandle EffectTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	UParticleSystem* ParticleEffect = nullptr;

	UParticleSystemComponent* ParticleEmitter = nullptr;
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_SpeedUpEffect : public UTPS_StatsEffects
{
	GENERATED_BODY()

public:
	bool InitObject(AActor* ActorToSpeedUp) override;
	void DestroyObject() override;
	void IncreaseSpeed();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float speedUpCoef = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float speedUpTimer = 5.f;

	ATopDownShooterCharacter* pointerToCharacter = nullptr;
	FTimerHandle decreaseTimer;
};