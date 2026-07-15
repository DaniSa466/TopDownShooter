// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "TopDownShooterCharacter.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "TopDownShooter/Game/TopDownShooterGameInstance.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::SwitchWeaponToNextOrPrevious(int8 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward, bool CalledFromPickUp)
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
		{
			NewIndex = InventoryPointerToCharacter->GetCurrentWeaponIndex();
			InventoryPointerToCharacter = nullptr;
		}
	}

	if (SwitchIsSuccess)
	{
		SetAdditionalWeaponInfo(OldIndex, OldInfo);
		SwitchWeaponEvent_OnServer(WeaponSlots[NewIndex].NameItem, 
			WeaponSlots[NewIndex].AdditionalInfo, NewIndex);
	}

	return SwitchIsSuccess;
}

bool UInventoryComponent::SwitchWeaponToIndex(int32 indexWeaponToChange, int32 previousIndex, FAdditionalWeaponInfo previousWeaponInfo)
{
	bool isSuccess = false;
	FName weaponNameToSwitch;
	FAdditionalWeaponInfo AdditionalInfoToSwitch;

	weaponNameToSwitch = GetWeaponNameByIndexSlot(indexWeaponToChange);
	AdditionalInfoToSwitch = GetAdditionalWeaponInfo(indexWeaponToChange);

	if (!weaponNameToSwitch.IsNone())
	{
		SetAdditionalWeaponInfo(previousIndex, previousWeaponInfo);
		SwitchWeaponEvent_OnServer(weaponNameToSwitch, AdditionalInfoToSwitch, indexWeaponToChange);

		//check ammo slot for event to player
		EWeaponType weaponTypeToSwitch;
		if (GetWeaponTypeByWeaponName(weaponNameToSwitch, weaponTypeToSwitch))
		{
			int16 avialableAmmoForWeapon = -1;
			if (CheckAmmoForWeapon(weaponTypeToSwitch, avialableAmmoForWeapon))
			{ }
		}
		isSuccess = true;
	}
	return isSuccess;
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

	if (ATopDownShooterCharacter* InventoryPointerToCharacter = Cast<ATopDownShooterCharacter>(GetOwner()))
	{
		if (InventoryPointerToCharacter->GetCurrentWeapon() &&
			InventoryPointerToCharacter->GetCurrentWeapon()->AdditionalWeaponInfo.Round == 0)
			AmmoEmptyEvent_Multicast(WeaponType); //visual sign for empty ammo

		InventoryPointerToCharacter = nullptr;
	}

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

bool UInventoryComponent::GetWeaponTypeByIndexSlot(int32 indexSlot, EWeaponType& weaponType)
{
	bool isFound = false;
	FWeaponInfo outInfo;
	weaponType = EWeaponType::RifleType;
	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

	if (myGI)
	{
		myGI->GetWeaponInfoByName(WeaponSlots[indexSlot].NameItem, outInfo);
		weaponType = outInfo.WeaponType;
		isFound = true;
	}

	return isFound;
}

bool UInventoryComponent::GetWeaponTypeByWeaponName(FName weaponName, EWeaponType& weaponType)
{
	bool isFound = false;
	FWeaponInfo outInfo;
	weaponType = EWeaponType::RifleType;
	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

	if (myGI)
	{
		myGI->GetWeaponInfoByName(weaponName, outInfo);
		weaponType = outInfo.WeaponType;
		isFound = true;
	}

	return isFound;
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

				WeaponChangeAdditionalInfo_Multicast(i, NewInfo);
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

			AmmoChangeEvent_Multicast(AmmoSlots[i].WeaponType, AmmoSlots[i].count, TakenAmmo > 0);
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

		SwitchWeaponToNextOrPrevious(WeaponIndexToChange, NewWeapon.AdditionalInfo, false, true);
		UpdateWeaponSlotsEvent_Multicast(WeaponIndexToChange, NewWeapon);

		result = true;
	}

	return result;
}

void UInventoryComponent::DropWeaponByIndex_OnServer_Implementation(int32 index)
{
	FDropItem dropItemInfo;
	FWeaponSlot emptyWeaponSlot;

	bool canBeDropped = false;
	int8 i = 0;
	int8 avialableWeaponNum = 0;
	while (i < WeaponSlots.Num() && !canBeDropped)
	{
		if (!WeaponSlots[i].NameItem.IsNone())
		{
			avialableWeaponNum++;
			if (avialableWeaponNum > 1)
				canBeDropped = true;
		}
		i++;
	}

	if (canBeDropped && WeaponSlots.IsValidIndex(index) && GetDropItemFromInventory(index, dropItemInfo))
	{
		//switch weapon to valid slot from start weaponSlots array
		bool bWeaponIsFound = false;
		int8 j = 0;
		while (j < WeaponSlots.Num() && !bWeaponIsFound)
		{
			if (!WeaponSlots[j].NameItem.IsNone() && j != index)
			{
				SwitchWeaponToIndex(j, index, WeaponSlots[index].AdditionalInfo);
				SwitchWeaponEvent_OnServer(WeaponSlots[j].NameItem, WeaponSlots[j].AdditionalInfo, j);
				bWeaponIsFound = true;
			}

			j++;
		}

		WeaponSlots[index] = emptyWeaponSlot;
		if (GetOwner()->GetClass()->ImplementsInterface(UTPS_GameActorsInterface::StaticClass()))
		{
			ITPS_GameActorsInterface::Execute_DropWeaponToWorld(GetOwner(), dropItemInfo);
		}

		UpdateWeaponSlotsEvent_Multicast(index, emptyWeaponSlot);
	}
}

void UInventoryComponent::TryGetWeaponToInventory_OnServer_Implementation(
	AActor* pickUpActor, FWeaponSlot NewWeapon)
{
	bool bCanTake = false;
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

	if (CheckCanTakeWeapon(IndexSlot) && !WeaponIsInInventory)
	{
		WeaponSlots[IndexSlot] = NewWeapon;
		UpdateWeaponSlotsEvent_Multicast(IndexSlot, NewWeapon);

		if (pickUpActor)
			pickUpActor->Destroy();

		bCanTake = true;
	}
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

TArray<FWeaponSlot> UInventoryComponent::GetWeaponSlots()
{
	return WeaponSlots;
}

TArray<FAmmoSlot> UInventoryComponent::GetAmmoSlots()
{
	return AmmoSlots;
}

void UInventoryComponent::InitInventory_OnServer_Implementation(const TArray<FWeaponSlot>& newWeaponSlotsInfo, const TArray<FAmmoSlot>& newAmmoSlotsInfo)
{
	WeaponSlots = newWeaponSlotsInfo;
	AmmoSlots = newAmmoSlotsInfo;

	//Find Init WeaponSlots and First Init Weapon

	MaxSlotsWeapon = WeaponSlots.Num();

	if (WeaponSlots.IsValidIndex(0))
		if (!WeaponSlots[0].NameItem.IsNone())
			SwitchWeaponEvent_OnServer(WeaponSlots[0].NameItem, WeaponSlots[0].AdditionalInfo, 0);
}

void UInventoryComponent::SwitchWeaponEvent_OnServer_Implementation(FName idWeaponName, 
	FAdditionalWeaponInfo additionalWeaponInfo, int32 newCurrentIndexWeapon)
{
	OnSwitchWeapon.Broadcast(idWeaponName, additionalWeaponInfo, newCurrentIndexWeapon);
}

void UInventoryComponent::AmmoChangeEvent_Multicast_Implementation(EWeaponType typeWeapon, int32 count, bool isIncreasing)
{
	OnAmmoChange.Broadcast(typeWeapon, count, isIncreasing);
}

void UInventoryComponent::WeaponChangeAdditionalInfo_Multicast_Implementation(int32 indexSlot, FAdditionalWeaponInfo additionalInfo)
{
	OnWeaponChangeAdditionalInfo.Broadcast(indexSlot, additionalInfo);
}

void UInventoryComponent::AmmoEmptyEvent_Multicast_Implementation(EWeaponType weaponType)
{
	OnAmmoEmpty.Broadcast(weaponType);
}

void UInventoryComponent::AmmoAvialableEvent_Multicast_Implementation(EWeaponType weaponType)
{
	OnAmmoAvialable.Broadcast(weaponType);
}

void UInventoryComponent::UpdateWeaponSlotsEvent_Multicast_Implementation(int32 indexSlot, FWeaponSlot newInfo)
{
	OnUpdateWeaponSlots.Broadcast(indexSlot, newInfo);
}

void UInventoryComponent::WeaponHasNoRoundEvent_Multicast_Implementation(int32 weaponIndex)
{
	OnWeaponHasNoRound.Broadcast(weaponIndex);
}

void UInventoryComponent::WeaponHasRoundEvent_Multicast_Implementation(int32 weaponIndex)
{
	OnWeaponHasRound.Broadcast(weaponIndex);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, WeaponSlots);
	DOREPLIFETIME(UInventoryComponent, AmmoSlots);

}