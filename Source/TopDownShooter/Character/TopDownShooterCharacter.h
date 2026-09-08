// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "TopDownShooter/Weapon/WeaponDefault.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "TopDownShooterCharacter.generated.h"

class UTPS_StatsEffects;
class UTPS_TemporaryEffect;
class UTPS_CharHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnableSpeedUpEffect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisableSpeedUpEffect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireBulletsEffectEnable, int32, weaponIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireBulletsEffectDisable);

UCLASS(Blueprintable)
class ATopDownShooterCharacter : public ACharacter, public ITPS_GameActorsInterface
{
	GENERATED_BODY()

protected:
	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
		FReplicationFlags* RepFlags) override;

	virtual void BeginPlay() override;

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	//input flags
	float AxisX = 0.0f, AxisY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool WalkEnabled = false;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool AimEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool SprintRunEnabled = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState MovementState = EMovementState::Stand_State;
	UPROPERTY(Replicated)
	AWeaponDefault* CurrentWeapon = nullptr;
	UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly)
	int32 CurrentIndexWeapon = 0;

	UDecalComponent* CurrentCursor = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, EditDefaultsOnly)
	TArray<UTPS_StatsEffects*> Effects;
	UPROPERTY(ReplicatedUsing = OnRep_EffectToAdd)
	UTPS_StatsEffects* effectToAdd = nullptr;
	UPROPERTY(ReplicatedUsing = OnRep_EffectToRemove)
	UTPS_StatsEffects* effectToRemove = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TArray<UParticleSystemComponent*> particleSystemEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UTPS_StatsEffects> burnEffect;

	//inputs
	void InputAxisX(float Value);
	void InputAxisY(float Value);

	void InputAttackPressed();
	void InputAttackReleased();

	//Inventory functions
	UFUNCTION(Server, Reliable)
	void SwitchNextWeapon_OnServer();
	UFUNCTION(Server, Reliable)
	void SwitchPreviousWeapon_OnServer();

	//ability
	void TryAbilityEnabled();

	template <int32 id>
	void TKeyPressed()
	{
		TrySwitchWeaponToIndexByKeyInput_OnServer(id);
	}

	//Health functions
	UFUNCTION()
	void CharDead();
	UFUNCTION(NetMulticast, Reliable)
	void EnableRagDoll_Multicast();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

public:
	ATopDownShooterCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* NewInputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UTPS_CharHealthComponent* HealthComponent;

private:
	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom; 

	//coef for effects
	float speedUpCoef = 1.f;
	bool isFireBulletEffect = false;
	FTimerHandle fireBulletsEffectTimerHandle;

	// stun
	UAnimMontage* stunAnimation = nullptr;
	UParticleSystem* stunEffect = nullptr;
	UParticleSystemComponent* stunEmitter = nullptr;

public:
	//delegates
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnEnableSpeedUpEffect OnEnableSpeedUpEffect;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnDisableSpeedUpEffect OnDisableSpeedUpEffect;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnFireBulletsEffectEnable OnFireBulletsEffectEnable;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnFireBulletsEffectDisable OnFireBulletsEffectDisable;

	//variables
	//cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);

	//movement system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed MovementSpeedInfo;

	//Stamina system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int Stamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	int MaxStamina = 100;

	//Forward sprinting variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int MaxDeviation = 20;

	float ResSpeed = 600.f;
	
	int deviation;
	double DeviationCos;
	double DeviationRad;
	bool NormalDeviation;

	FVector LookingDirection;
	FVector MovingDirection;

	//Health variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	TArray<UAnimMontage*> DeadAnimations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	TSubclassOf<UTPS_StatsEffects> AbilityEffect;

	FTimerHandle RagDollTimer;

	//for demo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponName;

	//getters and setters for effects
	void SetSpeedCoef(float newCoef = 1.f);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDecalComponent* GetCursorToWorld();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponDefault* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	EMovementState GetMovementState();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UTPS_StatsEffects*> GetCurrentEffectsOnChar();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetCurrentWeaponIndex();
	UFUNCTION(BlueprintCallable)
	bool GetIsAlive();
	//end getters and setters for effects

	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();

	//Tick function
	UFUNCTION()
	void MovementTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();

	UFUNCTION(BlueprintCallable)
	void ChangeMovementState();

	UFUNCTION(BlueprintCallable)
	void AttackCharEvent(bool bIsFiring);

	void StaminaSystem(EMovementState State);

	void SprintDirectionLimitation(EMovementState State);

	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo AdditoinalWeaponInfo, int32 NewCurrentIndexWeapon);

	UFUNCTION(Server, Reliable)
	void TrySwitchWeaponToIndexByKeyInput_OnServer(int32 index);
	void DropCurrentWeapon();

	UFUNCTION()
	void WeaponFire(UAnimMontage* Anim);

	UFUNCTION()
	void WeaponReloadStart(UAnimMontage* Anim);
	
	UFUNCTION()
	void WeaponReloadEnd(bool bIsSuccess, int32 AmmoTake);

	UFUNCTION(BlueprintNativeEvent)
	void WeaponFire_BP(UAnimMontage* Anim);

	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadStart_BP(UAnimMontage* Anim);

	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadEnd_BP(bool bIsSuccess);

	UFUNCTION(BlueprintNativeEvent)
	void CharDead_BP();

	//Interface
	EPhysicalSurface GetSurfaceType() override;
	TArray<UTPS_StatsEffects*> GetCurrentEffects() override;
	void RemoveEffect(UTPS_StatsEffects* effectToRemove) override;
	void AddEffect(UTPS_StatsEffects* effectToAdd) override;

	// multiplayer
	// effects
	UFUNCTION()
	void OnRep_EffectToAdd();
	UFUNCTION()
	void OnRep_EffectToRemove();
	UFUNCTION(Server, Reliable)
	void ExecuteEffectAdd_OnServer(UParticleSystem* effectFX);
	UFUNCTION(NetMulticast, Reliable)
	void ExecuteEffectAdd_Multicast(UParticleSystem* effectFX);
	UFUNCTION()
	void SwitchEffect(UTPS_StatsEffects* newEffect, bool bIsAdd);

	// base
	UFUNCTION(Server, Unreliable)
	void SetActorRotationByYaw_OnServer(float yaw);
	UFUNCTION(NetMulticast, Unreliable)
	void SetActorRotationByYaw_Multicast(float yaw);

	// effects
	UFUNCTION(NetMulticast, Reliable)
	void ChangeCharacterInputStatus_Multicast(bool isStun, UAnimMontage* loopAnimation, 
		UParticleSystem* ParticleEffect, UParticleSystemComponent* ParticleEmitter);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void EnableFireBulletsEffect_OnServer();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void DisableFireBulletsEffect_OnServer();

	UFUNCTION(Client, Unreliable)
	void EnableFireBulletsEffect_OnClient(int32 fireBulletsWeaponIndex);
	UFUNCTION(Client, Unreliable)
	void DisableFireBulletsEffect_OnClient();

	UFUNCTION(Server, Reliable)
	void SetMovementState_OnServer(EMovementState newState);
	UFUNCTION(NetMulticast, Reliable)
	void SetMovementState_Multicast(EMovementState newState);
	UFUNCTION(Server, Reliable)
	void TryReloadWeapon_OnServer();
	UFUNCTION(NetMulticast, Unreliable)
	void PlayAnim_Multicast(UAnimMontage* anim);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};