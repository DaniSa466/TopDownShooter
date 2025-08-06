// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "TopDownShooter/Weapon/WeaponDefault.h"
#include "TopDownShooterCharacter.generated.h"

UCLASS(Blueprintable)
class ATopDownShooterCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

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

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	//variables
	//cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);

	//movement system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState MovementState = EMovementState::Stand_State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool WalkEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool AimEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool SprintRunEnabled = false;

	//Stamina system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	int Stamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
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

	//weapon
	AWeaponDefault* CurrentWeapon = nullptr;

	//for demo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponName;

	UDecalComponent* CurrentCursor = nullptr;

	//inputs
	UFUNCTION()
	void InputAxisX(float Value);

	UFUNCTION()
	void InputAxisY(float Value);

	float AxisX = 0.0f, AxisY = 0.0f;

	UFUNCTION()
	void InputAttackPressed();

	UFUNCTION()
	void InputAttackReleased();

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

	UFUNCTION()
	void StaminaSystem(EMovementState State);

	UFUNCTION()
	void SprintDirectionLimitation(EMovementState State);

	UFUNCTION(BlueprintCallable)
	AWeaponDefault* GetCurrentWeapon();

	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo AdditoinalWeaponInfo, int32 NewCurrentIndexWeapon);

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

	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld();

	//Inventory functions
	void SwitchNextWeapon();
	void SwitchPreviousWeapon();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 CurrentIndexWeapon = 0;
};