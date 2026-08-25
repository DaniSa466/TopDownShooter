// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "TPS_EnemyCharacter.generated.h"

class UTPS_StatsEffects;

UCLASS()
class TOPDOWNSHOOTER_API ATPS_EnemyCharacter : public ACharacter, public ITPS_GameActorsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPS_EnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
		FReplicationFlags* RepFlags) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void RemoveEffect(UTPS_StatsEffects* effectToRemove) override;
	void AddEffect(UTPS_StatsEffects* effectToAdd) override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	TArray<UTPS_StatsEffects*> Effects;
	UPROPERTY(ReplicatedUsing = OnRep_EffectAdd)
	UTPS_StatsEffects* effectAdd = nullptr;
	UPROPERTY(ReplicatedUsing = OnRep_EffectRemove)
	UTPS_StatsEffects* effectRemove = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TArray<UParticleSystemComponent*> particleSystemEffects;

	UFUNCTION()
	void OnRep_EffectAdd();
	UFUNCTION()
	void OnRep_EffectRemove();

	UFUNCTION(Server, Reliable)
	void ExecuteEffectAdd_OnServer(UParticleSystem* effectFX);
	UFUNCTION(NetMulticast, Reliable)
	void ExecuteEffectAdd_Multicast(UParticleSystem* effectFX);

	UFUNCTION()
	void SwitchEffect(UTPS_StatsEffects* effect, bool bIsAdd);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
