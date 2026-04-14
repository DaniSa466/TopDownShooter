// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPS_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChange, float, Health, float, damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

USTRUCT(BlueprintType)
struct FStatsParam
{
	GENERATED_BODY()


};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOPDOWNSHOOTER_API UTPS_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FTimerHandle startRegen_TimerHandle;
	FTimerHandle regen_TimerHandle;
	float timeToStartRegen = 10.f, timeToRegen = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Health")
	bool bCanRegen = false;

	void RegenHealth();
	void StartRegen();

public:	
	// Sets default values for this component's properties
	UTPS_HealthComponent();

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Health")
	FOnHealthChange OnHealthChange;
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Health")
	FOnDead OnDead;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool resistToDamage = false;

	float maxHealth = 100.f;
	float HealthValue = 100.f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float DamageCoef = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool GetResistToDamage();
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ChangeResistToDamage();

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth();
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float SetHealth);
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void ChangeCurrentHealth(float ChangeValue);
};