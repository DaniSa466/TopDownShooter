// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
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
				else
				{
					WeaponSlots.RemoveAt(i);
					i--;
				}
			}
	}

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

bool UInventoryComponent::SwitchWeaponToIndex(int8 NewIndex, int8 OldIndex, FAdditionalWeaponInfo OldInfo)
{
	bool bIsSuccess = false;
	int8 CorrectIndex;

	//checking if NewIndex is out of range
	if (NewIndex > WeaponSlots.Num() - 1)
		CorrectIndex = 0;
	if (NewIndex < 0)
		CorrectIndex = WeaponSlots.Num() - 1;

	FName NewIdWeapon;
	FAdditionalWeaponInfo NewAdditionalInfo;
	
	int8 i = 0;
	while (i <= WeaponSlots.Num() - 1 && !bIsSuccess)
	{
		if (i == CorrectIndex)
		{
			if (!WeaponSlots[i].NameItem.IsNone())
			{
				NewIdWeapon = WeaponSlots[i].NameItem;
				NewAdditionalInfo = WeaponSlots[i].AdditionalInfo;
				bIsSuccess = true;
			}
		}
		i++;
	}

	if (!bIsSuccess)
	{
		//Weapon Switch isn't seccess
	}

	if (bIsSuccess)
	{
		SetAdditionalWeaponInfo(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(NewIdWeapon, NewAdditionalInfo, CorrectIndex);
	}

	return bIsSuccess;
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

void UInventoryComponent::SetAdditionalWeaponInfo(int8 WeaponIndex, FAdditionalWeaponInfo NewInfo)
{
	if (WeaponSlots.IsValidIndex(WeaponIndex))
	{
		bool bIsFound = false;
		int8 i = 0;
		while (i <= WeaponSlots.Num() - 1 && !bIsFound)
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

void UInventoryComponent::WeaponChangeAmmo(EWeaponType TypeWeapon, int32 TakenAmmo)
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

