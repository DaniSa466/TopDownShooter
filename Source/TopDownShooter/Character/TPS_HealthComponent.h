// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPS_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChange, float, Health);
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

	float health = 100.f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float GetCurrentHealth();
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ReceveDamage(float damage);
	UFUNCTION(BlueprintNativeEvent)
	void DeadEvent_BP();
};
