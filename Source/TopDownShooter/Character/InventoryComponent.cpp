// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "TopDownShooterCharacter.h"
#include "TopDownShooter/Game/TopDownShooterGameInstance.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	//Find Init WeaponSlots and First Init Weapon
	for (int8 i = 0; i < WeaponSlots.Num(); i++)
	{
		UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());
		if (myGI)
			if (!WeaponSlots[i].NameItem.IsNone())
			{
				FWeaponInfo Info;
				if (myGI->GetWeaponInfoByName(WeaponSlots[i].NameItem, Info))
					WeaponSlots[i].AdditionalInfo.Round = Info.MaxRound;
			}
	}

	MaxSlotsWeapon = WeaponSlots.Num(); 

	if (WeaponSlots.IsValidIndex(0))
		if (!WeaponSlots[0].NameItem.IsNone())
			OnSwitchWeapon.Broadcast(WeaponSlots[0].NameItem, WeaponSlots[0].AdditionalInfo, 0);
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::SwitchWeaponToIndex(int8 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward, bool CalledFromPickUp)
{
	bool SwitchIsSuccess = false;
	int8 NewIndex = OldIndex;

	//main logic of switching
	if (bIsForward)
	{
		int8 i = 0;
		while (i < WeaponSlots.Num() && !SwitchIsSuccess)
		{
			NewIndex++;

			if (NewIndex == WeaponSlots.Num())
				NewIndex = 0;

			if (!WeaponSlots[NewIndex].NameItem.IsNone())
				if (WeaponSlots[NewIndex].AdditionalInfo.Round > 0)
					SwitchIsSuccess = true;

				//checking ammo for the weapon in inventory
				else
				{
					FWeaponInfo InfoToType;
					UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

					if (myGI)
					{
						myGI->GetWeaponInfoByName(WeaponSlots[NewIndex].NameItem, InfoToType);

						int8 j = 0;
						bool AmmoAreFound = false;
						while (j < AmmoSlots.Num() && !AmmoAreFound)
						{
							if (AmmoSlots[j].WeaponType == InfoToType.WeaponType && AmmoSlots[j].count > 0)
							{
								SwitchIsSuccess = true;
								AmmoAreFound = true;
							}

							j++;
						}
					}
				}
			i++;
		}
	}

	else
	{
		int8 i = 0;
		while (i < WeaponSlots.Num() && !SwitchIsSuccess)
		{
			NewIndex--;

			if (NewIndex == -1)
				NewIndex = WeaponSlots.Num() - 1;

			if (!WeaponSlots[NewIndex].NameItem.IsNone())
				if (WeaponSlots[NewIndex].AdditionalInfo.Round > 0)
					SwitchIsSuccess = true;
				else
				{
					FWeaponInfo InfoToType;
					UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

					//checking ammo for the weapon in inventory
					if (myGI)
					{
						myGI->GetWeaponInfoByName(WeaponSlots[NewIndex].NameItem, InfoToType);

						int8 j = 0;
						bool AmmoAreFound = false;
						while (j < AmmoSlots.Num() && !AmmoAreFound)
						{
							if (AmmoSlots[j].WeaponType == InfoToType.WeaponType && AmmoSlots[j].count > 0)
							{
								SwitchIsSuccess = true;
								AmmoAreFound = true;
							}

							j++;
						}
					}
				}
			i++;
		}
	}

	if (CalledFromPickUp)
	{ 
		if (ATopDownShooterCharacter* InventoryPointerToCharacter = Cast<ATopDownShooterCharacter>(GetOwner()))
			NewIndex = InventoryPointerToCharacter->CurrentIndexWeapon;
	}

	if (SwitchIsSuccess)
	{
		SetAdditionalWeaponInfo(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(WeaponSlots[NewIndex].NameItem, WeaponSlots[NewIndex].AdditionalInfo, NewIndex);
	}

	return SwitchIsSuccess;
}

bool UInventoryComponent::CheckAmmoForWeapon(EWeaponType WeaponType, int16 &AvialableAmmoForWeapon)
{
	AvialableAmmoForWeapon = 0;
	bool bIsFound = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !bIsFound)
	{
		if (AmmoSlots[i].WeaponType == WeaponType)
		{
			bIsFound = true;
			AvialableAmmoForWeapon = AmmoSlots[i].count;
			if (AmmoSlots[i].count > 0)
				return true;
		}
		i++;
	}

	OnAmmoEmpty.Broadcast(WeaponType); //visual sign for empty ammo

	return false;
}

FAdditionalWeaponInfo UInventoryComponent::GetAdditionalWeaponInfo(int8 WeaponIndex)
{
	FAdditionalWeaponInfo result;
	if (WeaponSlots.IsValidIndex(WeaponIndex))
	{
		bool bIsFound = false;
		int8 i = 0;
		while (i < WeaponSlots.Num() && !bIsFound)
		{
			if (i == WeaponIndex)
			{
				result = WeaponSlots[i].AdditionalInfo;
				bIsFound = true;
			}
			i++;
		}
		if (!bIsFound)
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::GetAdditionalWeaponInfo - Not found Weapon Index -%d"), WeaponIndex);
	}

	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::GetAdditionalWeaponInfo - Not correct Weapon Index -%d"), WeaponIndex);

	return result;
}

int8 UInventoryComponent::GetWeaponIndexSlotByName(FName WeaponName)
{
	int8 result = -1;
	int8 i = 0;
	bool bIsFound = false;
	while (i < WeaponSlots.Num() && !bIsFound)
	{
		if (WeaponSlots[i].NameItem == WeaponName)
		{
			result = i;
			bIsFound = true;
		}
		i++;
	}
	return result;
}

FName UInventoryComponent::GetWeaponNameByIndexSlot(int8 IndexSlot)
{
	FName result;

	if (WeaponSlots.IsValidIndex(IndexSlot))
		result = WeaponSlots[IndexSlot].NameItem;

	return result;
}

void UInventoryComponent::SetAdditionalWeaponInfo(int8 WeaponIndex, FAdditionalWeaponInfo NewInfo)
{
	if (WeaponSlots.IsValidIndex(WeaponIndex))
	{
		bool bIsFound = false;
		int8 i = 0;
		while (i <= WeaponSlots.Num() && !bIsFound)
		{
			if (i == WeaponIndex)
			{
				WeaponSlots[i].AdditionalInfo = NewInfo;
				bIsFound = true;

				OnWeaponChangeAdditionalInfo.Broadcast(i, NewInfo);
			}
			i++;
		}

		if (!bIsFound)
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - Not found weapon with index - %d"), WeaponIndex);
	}

	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SetAdditionalWeaponInfo - Not correct Weapon Index -%d"), WeaponIndex);
}

void UInventoryComponent::AmmoSlotChangeValue(EWeaponType TypeWeapon, int32 TakenAmmo)
{
	int8 i = 0;
	bool bIsFound = false;

	while (i < AmmoSlots.Num() && !bIsFound)
	{
		if (AmmoSlots[i].WeaponType == TypeWeapon)
		{
			AmmoSlots[i].count += TakenAmmo;

			if (AmmoSlots[i].count > AmmoSlots[i].MaxCount)
				AmmoSlots[i].count = AmmoSlots[i].MaxCount;

			OnAmmoChange.Broadcast(AmmoSlots[i].WeaponType, AmmoSlots[i].count);
			bIsFound = true;	
		}

		i++;
	}
}
bool UInventoryComponent::CheckCanTakeAmmo(EWeaponType AmmoType)
{
	bool result = false;
	int8 i = 0;
	while (i < AmmoSlots.Num() && !result)
	{
		if (AmmoSlots[i].WeaponType == AmmoType && AmmoSlots[i].count < AmmoSlots[i].MaxCount)
			result = true;
		i++;
	}
	return result;
}

bool UInventoryComponent::CheckCanTakeWeapon(int32 &FreeSlot)
{
	bool FreeSlotIsFound = false;  
	int8 i = 0;

	while (i < WeaponSlots.Num() && !FreeSlotIsFound)
	{
		if (WeaponSlots[i].NameItem.IsNone())
		{
			FreeSlotIsFound = true;
			FreeSlot = i;
		}
		i++;
	}

	return FreeSlotIsFound;
}

bool UInventoryComponent::PickUpWeapon(FWeaponSlot NewWeapon, int32 WeaponIndexToChange, int32 CurrentWeaponIndex, FDropItem &DropItemInfo)
{
	bool result = false;
	if (GetDropItemFromInventory(WeaponIndexToChange, DropItemInfo))
	{
		WeaponSlots[WeaponIndexToChange] = NewWeapon;

		SwitchWeaponToIndex(WeaponIndexToChange, NewWeapon.AdditionalInfo, false, true);
		OnUpdateWeaponSlots.Broadcast(WeaponIndexToChange, NewWeapon);

		result = true;
	}
	//UE_LOG(LogTemp, Warning, TEXT("InventoryComponent::PickUpWeapon - SlotToChange = %f. Name = %f. Round = %f."), WeaponIndexToChange, NewWeapon.NameItem, NewWeapon.AdditionalInfo.Round);

	return result;
}

bool UInventoryComponent::TryGetWeaponToInventory(FWeaponSlot NewWeapon, bool &BPWeaponIsInInventory)
{
	bool CanTake = false;
	int32 IndexSlot = -1;
	
	// Check if inventory alredy has this weapon
	bool WeaponIsInInventory = false;
	int8 i = 0;

	while (i < WeaponSlots.Num() && !WeaponIsInInventory)
	{
		if (WeaponSlots[i].NameItem == NewWeapon.NameItem)
			WeaponIsInInventory = true;
		i++;
	}
	BPWeaponIsInInventory = WeaponIsInInventory;

	if (CheckCanTakeWeapon(IndexSlot) && !WeaponIsInInventory)
	{
		WeaponSlots[IndexSlot] = NewWeapon;
		OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
		CanTake = true;
	}
	return CanTake;
}

bool UInventoryComponent::GetDropItemFromInventory(int32 WeaponIndexToDrop, FDropItem &DropItemInfo)
{
	bool result;
	result = WeaponSlots.IsValidIndex(WeaponIndexToDrop);

	bool bCanDrop = false;
	FName DropItemName = GetWeaponNameByIndexSlot(WeaponIndexToDrop);
	DropItemInfo.WeaponInfo.NameItem = DropItemName;

	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());
	if (myGI)
	{
		bCanDrop = myGI->GetDropItemInfoByWeaponName(DropItemName, DropItemInfo);
		DropItemInfo.WeaponInfo.AdditionalInfo = WeaponSlots[WeaponIndexToDrop].AdditionalInfo;
	}

	return result && bCanDrop;
}
