// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "TPS_StatsEffects.generated.h"

/**
 * 
 */
class ATopDownShooterCharacter;
class UTPS_CharHealthComponent;


UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_StatsEffects : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	TArray <TEnumAsByte<EPhysicalSurface>> PossibleInteractSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool bIsStackable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool isAutoDestroy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun Effect Setting")
	UParticleSystem* ParticleEffect = nullptr;
	UParticleSystemComponent* ParticleEmitter = nullptr;

	AActor* newActor = nullptr;

	UPROPERTY(Replicated)
	FName boneName;

	bool IsSupportedForNetworking() const override { return true; };
	virtual bool InitObject(AActor* ActorToInit, FName hitBoneName);
	virtual void DestroyObject();

	virtual bool ChackStackableEffect();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_EffectExecuteOnce : public UTPS_StatsEffects
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Once Setting")
	float Power = 20.f;

public:
	bool InitObject(AActor* ActorToInit, FName hitBoneName) override;
	void DestroyObject() override;
	void ExecuteOnce();
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_TemporaryEffect : public UTPS_StatsEffects
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float Power = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float Timer = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Execute Timer Setting")
	float RateTime = 1.f;

	FTimerHandle ExecuteTimer;
	FTimerHandle EffectTimer;

public:
	bool InitObject(AActor* ActorToInit, FName hitBoneName) override;
	void DestroyObject() override;
	void Execute();
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_SpeedUpEffect : public UTPS_StatsEffects
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedUp Effect Setting")
	float speedUpCoef = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedUp Effect Setting")
	float speedUpTimer = 5.f;

	ATopDownShooterCharacter* pointerToCharacter = nullptr;
	FTimerHandle decreaseTimer;

public:
	bool InitObject(AActor* ActorToSpeedUp, FName hitBoneName) override;
	void DestroyObject() override;
	void IncreaseSpeed();
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_EffectsToHealth : public UTPS_StatsEffects
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect to Health Setting")
	float healthCoef = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect to Health Setting")
	float timer = 5;

	ATopDownShooterCharacter* pointerToCharacter = nullptr;
	UTPS_CharHealthComponent* pointerToHealthComponent = nullptr;
	FTimerHandle backTimer;

public:
	bool InitObject(AActor* actorToInit, FName hitBoneName) override;
	void DestroyObject() override;
	void ChangeHealthCoef();
};

UCLASS(Blueprintable, BlueprintType)
class TOPDOWNSHOOTER_API UTPS_StunEffect : public UTPS_StatsEffects
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun Effect Setting")
	UAnimMontage* loopAnimation = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stun Effect Setting")
	float timer = 3.f;

	ATopDownShooterCharacter* pointerToCharacter = nullptr;
	FTimerHandle stunTimer;

public:
	bool InitObject(AActor* ActorToStun, FName hitBoneName) override;
	void DestroyObject() override;
};