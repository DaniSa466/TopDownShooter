// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSwitchWeapon, FName, IdWeaponName, FAdditionalWeaponInfo, AdditionalWeaponInfo, int32, NewCurrentIndexWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChange, EWeaponType, AmmoType, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChangeAdditionalInfo, int32, IndexSlot, FAdditionalWeaponInfo, AditionalInfo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOPDOWNSHOOTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	FOnSwitchWeapon OnSwitchWeapon;	
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FOnAmmoChange OnAmmoChange;
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FOnWeaponChangeAdditionalInfo OnWeaponChangeAdditionalInfo;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FAmmoSlot> AmmoSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	int32 MaxSlotsWeapon = 0;

	bool SwitchWeaponToIndex(int8 NewIndex, int8 OldIndex, FAdditionalWeaponInfo OldInfo);

	FAdditionalWeaponInfo GetAdditionalWeaponInfo(int8 WeaponIndex);
	int8 GetWeaponIndexSlotByName(FName WeaponName);
	void SetAdditionalWeaponInfo(int8 WeaponIndex, FAdditionalWeaponInfo NewInfo);
	void WeaponChangeAmmo(EWeaponType TypeWeapon, int32 TakenAmmo);
};
