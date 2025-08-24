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

bool UInventoryComponent::SwitchWeaponToIndex(int8 NewIndex, int8 OldIndex, FAdditionalWeaponInfo OldInfo, bool bIsForward)
{
	bool bIsSuccess = false;
	int8 CorrectIndex;

	//checking if NewIndex is out of range
	if (NewIndex == WeaponSlots.Num())
		CorrectIndex = 0;
	else if (NewIndex == -1)
		CorrectIndex = WeaponSlots.Num() - 1;
	else
		CorrectIndex = NewIndex;

	FName NewIdWeapon;
	FAdditionalWeaponInfo NewAdditionalInfo;
	int32 NewCurrentIndex = 0;
	
	if (WeaponSlots.IsValidIndex(CorrectIndex))
	{
		if (!WeaponSlots[CorrectIndex].NameItem.IsNone())
		{
			if (WeaponSlots[CorrectIndex].AdditionalInfo.Round > 0)
				//weapon have ammo, start changing
				bIsSuccess = true;
			else
			{
				UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());
				if (myGI)
				{
					//check AmmoSlots for this weapon
					FWeaponInfo myInfo;
					myGI->GetWeaponInfoByName(WeaponSlots[CorrectIndex].NameItem, myInfo);

					bool bIsFound = false;
					int8 i = 0;
					while (i < AmmoSlots.Num() && !bIsFound)
					{
						if (AmmoSlots[i].WeaponType == myInfo.WeaponType && AmmoSlots[i].count > 0)
						{
							//weapon have ammo start changing
							bIsSuccess = true;
							bIsFound = true;
						}
						i++;
					}
				}
			}

			if (bIsSuccess)
			{
				NewCurrentIndex = CorrectIndex;
				NewIdWeapon = WeaponSlots[CorrectIndex].NameItem;
				NewAdditionalInfo = WeaponSlots[CorrectIndex].AdditionalInfo;
			}
		}
	}

	if (!bIsSuccess)
	{
		if (bIsForward)
		{
			int8 iteration = 0, secondIteration = 0;
			while (iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				iteration++;
				int8 tmpIndex = NewIndex + iteration;
				if (WeaponSlots.IsValidIndex(tmpIndex))
				{
					if (!WeaponSlots[tmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[tmpIndex].AdditionalInfo.Round > 0)
						{
							//stop looking for
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
							NewCurrentIndex = tmpIndex;
						}
						else
						{
							FWeaponInfo myInfo;
							UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

							myGI->GetWeaponInfoByName(WeaponSlots[tmpIndex].NameItem, myInfo);

							bool bIsFound = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFound)
							{
								if (AmmoSlots[j].WeaponType == myInfo.WeaponType && AmmoSlots[j].count > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
									NewCurrentIndex = tmpIndex;
									bIsFound = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					if (OldIndex != secondIteration)
					{
						if (WeaponSlots.IsValidIndex(secondIteration))
						{
							if (!WeaponSlots[secondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[secondIteration].AdditionalInfo.Round > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[secondIteration].NameItem;
									NewAdditionalInfo = WeaponSlots[secondIteration].AdditionalInfo;
									NewCurrentIndex = secondIteration;
								}
								else
								{
									FWeaponInfo myInfo;
									UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

									myGI->GetWeaponInfoByName(WeaponSlots[secondIteration].NameItem, myInfo);

									bool bIsFound = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFound)
									{
										if (AmmoSlots[j].WeaponType == myInfo.WeaponType && AmmoSlots[j].count > 0)
										{
											//stop looking for
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[secondIteration].NameItem;
											NewAdditionalInfo = WeaponSlots[secondIteration].AdditionalInfo;
											NewCurrentIndex = secondIteration;
											bIsFound = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						if (WeaponSlots.IsValidIndex(secondIteration))
						{
							if (!WeaponSlots[secondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[secondIteration].AdditionalInfo.Round > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
								}
								else
								{
									FWeaponInfo myInfo;
									UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

									myGI->GetWeaponInfoByName(WeaponSlots[tmpIndex].NameItem, myInfo);

									bool bIsFound = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFound)
									{
										if (AmmoSlots[j].WeaponType == myInfo.WeaponType)
										{
											if (AmmoSlots[j].count > 0)
											{
												//WeaponGood, it means weapon do nothing
											}
											else
												//didn't find weapon avialable ammo, need to init Pistol with Infinity ammo
												UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SwitchWeaponToIndex - InitPistol is needed"));
										}
										j++;
									}
								}
							}
						}
					}
					secondIteration++;
				}
			}
		}
		else
		{
			int8 iteration = 0, secondIteration = WeaponSlots.Num() - 1;
			while (iteration < WeaponSlots.Num() && !bIsSuccess)
			{
				iteration++;
				int8 tmpIndex = NewIndex - iteration;
				if (WeaponSlots.IsValidIndex(tmpIndex))
				{
					if (!WeaponSlots[tmpIndex].NameItem.IsNone())
					{
						if (WeaponSlots[tmpIndex].AdditionalInfo.Round > 0)
						{
							//stop looking for
							bIsSuccess = true;
							NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
							NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
							NewCurrentIndex = tmpIndex;
						}
						else
						{
							FWeaponInfo myInfo;
							UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

							myGI->GetWeaponInfoByName(WeaponSlots[tmpIndex].NameItem, myInfo);

							bool bIsFound = false;
							int8 j = 0;
							while (j < AmmoSlots.Num() && !bIsFound)
							{
								if (AmmoSlots[j].WeaponType == myInfo.WeaponType && AmmoSlots[j].count > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
									NewCurrentIndex = tmpIndex;
									bIsFound = true;
								}
								j++;
							}
						}
					}
				}
				else
				{
					if (OldIndex != secondIteration)
					{
						if (WeaponSlots.IsValidIndex(secondIteration))
						{
							if (!WeaponSlots[secondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[secondIteration].AdditionalInfo.Round > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[secondIteration].NameItem;
									NewAdditionalInfo = WeaponSlots[secondIteration].AdditionalInfo;
									NewCurrentIndex = secondIteration;
								}
								else
								{
									FWeaponInfo myInfo;
									UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

									myGI->GetWeaponInfoByName(WeaponSlots[secondIteration].NameItem, myInfo);

									bool bIsFound = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFound)
									{
										if (AmmoSlots[j].WeaponType == myInfo.WeaponType && AmmoSlots[j].count > 0)
										{
											//stop looking for
											bIsSuccess = true;
											NewIdWeapon = WeaponSlots[secondIteration].NameItem;
											NewAdditionalInfo = WeaponSlots[secondIteration].AdditionalInfo;
											NewCurrentIndex = secondIteration;
											bIsFound = true;
										}
										j++;
									}
								}
							}
						}
					}
					else
					{
						if (WeaponSlots.IsValidIndex(secondIteration))
						{
							if (!WeaponSlots[secondIteration].NameItem.IsNone())
							{
								if (WeaponSlots[secondIteration].AdditionalInfo.Round > 0)
								{
									//stop looking for
									bIsSuccess = true;
									NewIdWeapon = WeaponSlots[tmpIndex].NameItem;
									NewAdditionalInfo = WeaponSlots[tmpIndex].AdditionalInfo;
								}
								else
								{
									FWeaponInfo myInfo;
									UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());

									myGI->GetWeaponInfoByName(WeaponSlots[tmpIndex].NameItem, myInfo);

									bool bIsFound = false;
									int8 j = 0;
									while (j < AmmoSlots.Num() && !bIsFound)
									{
										if (AmmoSlots[j].WeaponType == myInfo.WeaponType)
										{
											if (AmmoSlots[j].count > 0)
											{
												//WeaponGood, it means weapon do nothing
											}
											else
												//didn't find weapon avialable ammo, need to init Pistol with Infinity ammo
												UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::SwitchWeaponToIndex - InitPistol is needed"));
										}
										j++;
									}
								}
							}
						}
					}
					secondIteration--;
				}
			}
		}
	}

	if (bIsSuccess)
	{
		SetAdditionalWeaponInfo(OldIndex, OldInfo);
		OnSwitchWeapon.Broadcast(NewIdWeapon, NewAdditionalInfo, NewCurrentIndex);
	}

	return bIsSuccess;
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
	if (GetDropItemFropInventory(WeaponIndexToChange, DropItemInfo))
	{
		WeaponSlots[WeaponIndexToChange] = NewWeapon;

		SwitchWeaponToIndex(CurrentWeaponIndex, -1, NewWeapon.AdditionalInfo, false);
		OnUpdateWeaponSlots.Broadcast(WeaponIndexToChange, NewWeapon);

		result = true;
	}
	return result;
}

bool UInventoryComponent::TryGetWeaponToInventory(FWeaponSlot NewWeapon)
{
	int IndexSlot = -1;
	
	if (CheckCanTakeWeapon(IndexSlot))
		if (WeaponSlots.IsValidIndex(IndexSlot))
		{
			WeaponSlots[IndexSlot] = NewWeapon;
			OnUpdateWeaponSlots.Broadcast(IndexSlot, NewWeapon);
			return true;
		}
	return false;
}

bool UInventoryComponent::GetDropItemFropInventory(int32 WeaponIndexToDrop, FDropItem &DropItemInfo)
{
	bool result = false;
	if (WeaponSlots.IsValidIndex(WeaponIndexToDrop))
		result = true;

	bool bCanDrop = false;
	FName DropItemName = GetWeaponNameByIndexSlot(WeaponIndexToDrop);

	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetWorld()->GetGameInstance());
	if (myGI)
	{
		bCanDrop = myGI->GetDropItemInfoByName(DropItemName, DropItemInfo);
		DropItemInfo.WeaponInfo.AdditionalInfo = WeaponSlots[WeaponIndexToDrop].AdditionalInfo;
	}

	return result && bCanDrop;
}


