// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSwitchWeapon, FName, IdWeaponName, FAdditionalWeaponInfo, AdditionalWeaponInfo, int32, NewCurrentIndexWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChange, EWeaponType, AmmoType, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChangeAdditionalInfo, int32, IndexSlot, FAdditionalWeaponInfo, AditionalInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoEmpty, EWeaponType, WeaponType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoAvialable, EWeaponType, WeaponType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateWeaponSLots,int32, IndexSlot , FWeaponSlot, NewInfo);

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
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FOnAmmoEmpty OnAmmoEmpty;
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FOnAmmoAvialable OnAmmoAvialable;
	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FOnUpdateWeaponSLots OnUpdateWeaponSlots;

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

	bool SwitchWeaponToIndex(int8 NewIndex, int8 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward);
	bool CheckAmmoForWeapon(EWeaponType WeaponType, int16 &AvialableAmmoForWeapon);

	FAdditionalWeaponInfo GetAdditionalWeaponInfo(int8 WeaponIndex);
	int8 GetWeaponIndexSlotByName(FName WeaponName);
	FName GetWeaponNameByIndexSlot(int8 IndexSlot);
	void SetAdditionalWeaponInfo(int8 WeaponIndex, FAdditionalWeaponInfo NewInfo);

	UFUNCTION(BlueprintCallable)
	void AmmoSlotChangeValue(EWeaponType TypeWeapon, int32 AmmoToChange);

	//Interface PickUp Actors
	/*May be I'll try to make Pick Up logic whicj will differ from dictor's.
	I wanna try not to create separete function for weapon and ammo, and try to make one function with bool input param,
	wich will show with what Character Interact.*/
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool CheckCanTakeAmmo(EWeaponType AmmoType);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool CheckCanTakeWeapon(int32 &FreeSlot);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool PickUpWeapon(FWeaponSlot NewWeapon, int32 WeaponIndexToChange, int32 CurrentWeaponIndex, FDropItem &DropItemInfo);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool TryGetWeaponToInventory(FWeaponSlot NewWeapon);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool GetDropItemFropInventory(int32 WeaponIndexToDrop, FDropItem &DropItemInfo);
};
