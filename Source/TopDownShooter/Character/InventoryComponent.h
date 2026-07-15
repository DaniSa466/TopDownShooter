// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSwitchWeapon, FName, idWeaponName, 
	FAdditionalWeaponInfo, additionalWeaponInfo, int32, newCurrentIndexWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChange, EWeaponType, ammoType, 
	int32, count, bool, isIncreasing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChangeAdditionalInfo, int32, indexSlot, 
	FAdditionalWeaponInfo, aditionalInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoEmpty, EWeaponType, weaponType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoAvialable, EWeaponType, weaponType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateWeaponSLots, int32, indexSlot , 
	FWeaponSlot, newInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponHasNoRound, int32, weaponIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponHasRound, int32, weaponIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOPDOWNSHOOTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnSwitchWeapon OnSwitchWeapon;	
	//event on change ammo in slots by weaponType
	UPROPERTY(BlueprintAssignable,  Category = "Inventory")
	FOnAmmoChange OnAmmoChange;
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnWeaponChangeAdditionalInfo OnWeaponChangeAdditionalInfo;
	//Event Ammo slots after change still empty rounds
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnAmmoEmpty OnAmmoEmpty;
	//Event Ammo slots after chage have rounds
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnAmmoAvialable OnAmmoAvialable;
	//Event weapon was change by slotIndex
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnUpdateWeaponSLots OnUpdateWeaponSlots;

	//Event current weapon has no additional_Rounds 
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnWeaponHasNoRound OnWeaponHasNoRound;
	//event current weapon has addition_Rounds
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnWeaponHasRound OnWeaponHasRound;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<FAmmoSlot> AmmoSlots;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	int32 MaxSlotsWeapon = 0;

	//last variable is added correct working because without it
	//function switches weapon if player just picked up another
	bool SwitchWeaponToNextOrPrevious(int8 OldIndex, FAdditionalWeaponInfo OldInfo, 
		bool bIsForward, bool CalledFromPickUp = false);
	bool SwitchWeaponToIndex(int32 indexWeaponToChange, int32 previousIndex, FAdditionalWeaponInfo previousWeaponInfo);
	bool CheckAmmoForWeapon(EWeaponType WeaponType, int16 &AvialableAmmoForWeapon);

	void SetAdditionalWeaponInfo(int8 WeaponIndex, FAdditionalWeaponInfo NewInfo);
	FAdditionalWeaponInfo GetAdditionalWeaponInfo(int8 WeaponIndex);
	int8 GetWeaponIndexSlotByName(FName WeaponName);
	FName GetWeaponNameByIndexSlot(int8 IndexSlot);
	bool GetWeaponTypeByIndexSlot(int32 indexSlot, EWeaponType& weaponType);
	bool GetWeaponTypeByWeaponName(FName weaponName, EWeaponType& weaponType);

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
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "PickUpItems")
	void TryGetWeaponToInventory_OnServer(AActor* pickUpActor, FWeaponSlot NewWeapon);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool PickUpWeapon(FWeaponSlot NewWeapon, int32 WeaponIndexToChange, int32 CurrentWeaponIndex, FDropItem &DropItemInfo);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "PickUpItems")
	void DropWeaponByIndex_OnServer(int32 index);
	UFUNCTION(BlueprintCallable, Category = "PickUpItems")
	bool GetDropItemFromInventory(int32 WeaponIndexToDrop, FDropItem &DropItemInfo);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FWeaponSlot> GetWeaponSlots();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FAmmoSlot> GetAmmoSlots();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void InitInventory_OnServer(const TArray<FWeaponSlot>& newWeaponSlotsInfo, 
		const TArray<FAmmoSlot>& newAmmoSlotsInfo);

	// multicast functions for delegates
	UFUNCTION(Server, Reliable)
	void SwitchWeaponEvent_OnServer(FName idWeaponName,
		FAdditionalWeaponInfo additionalWeaponInfo, int32 newCurrentIndexWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void AmmoChangeEvent_Multicast(EWeaponType typeWeapon, int32 count, bool isIncreasing);
	UFUNCTION(NetMulticast, Reliable)
	void WeaponChangeAdditionalInfo_Multicast(int32 indexSlot, FAdditionalWeaponInfo aditionalInfo);
	UFUNCTION(NetMulticast, Reliable)
	void AmmoEmptyEvent_Multicast(EWeaponType weaponType);
	UFUNCTION(NetMulticast, Reliable)
	void AmmoAvialableEvent_Multicast(EWeaponType weaponType);
	UFUNCTION(NetMulticast, Reliable)
	void UpdateWeaponSlotsEvent_Multicast(int32 indexSlot, FWeaponSlot newInfo);
	UFUNCTION(NetMulticast, Reliable)
	void WeaponHasNoRoundEvent_Multicast(int32 weaponIndex);
	UFUNCTION(NetMulticast, Reliable)
	void WeaponHasRoundEvent_Multicast(int32 weaponIndex);
	// end delegates

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
