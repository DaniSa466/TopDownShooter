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
class UTPS_CharHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnableSpeedUpEffect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisableSpeedUpEffect);

UCLASS(Blueprintable)
class ATopDownShooterCharacter : public ACharacter, public ITPS_GameActorsInterface
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	//input flags
	float AxisX = 0.0f, AxisY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool WalkEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool AimEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool SprintRunEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState MovementState = EMovementState::Stand_State;
	
	AWeaponDefault* CurrentWeapon = nullptr;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 CurrentIndexWeapon = 0;

	UDecalComponent* CurrentCursor = nullptr;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<UTPS_StatsEffects*> Effects;

	//inputs
	void InputAxisX(float Value);
	void InputAxisY(float Value);

	void InputAttackPressed();
	void InputAttackReleased();

	//Inventory functions
	void SwitchNextWeapon();
	void SwitchPreviousWeapon();

	//ability
	void TryAbilityEnabled();

	template <int32 id>
	void TKeyPressed()
	{
		TrySwitchWeaponToIndexByKeyInput(id);
	}

	//Health functions
	UFUNCTION()
	void CharDead();
	void EnableRagDoll();
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
	bool resistToDamage = false;

public:
	//delegates
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnEnableSpeedUpEffect OnEnableSpeedUpEffect;

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FOnDisableSpeedUpEffect OnDisableSpeedUpEffect;

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
	bool IsAlive = true;
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

	UFUNCTION()
	void SetResistToDamage(bool resist = false);

	UFUNCTION(BlueprintCallable)
	bool GetResistToDamage();

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
	//end getters and setters for effects

	UTPS_CharHealthComponent* GetHealthComponent();

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

	bool TrySwitchWeaponToIndexByKeyInput(int32 index);
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

	//Interface
	EPhysicalSurface GetSurfaceType() override;
	TArray<UTPS_StatsEffects*> GetCurrentEffects() override;
	void RemoveEffect(UTPS_StatsEffects* EffectToRemove) override;
	void AddEffect(UTPS_StatsEffects* EffectToAdd) override;
};