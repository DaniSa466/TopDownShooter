// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownShooter/Game/TopDownShooterGameInstance.h"

bool UTopDownShooterGameInstance::GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo)
{
    bool bIsFind = false;
    FWeaponInfo* WeaponInfoRow;

    if (WeaponInfoTable)
    {
        WeaponInfoRow = WeaponInfoTable->FindRow<FWeaponInfo>(NameWeapon, "", false);
        if (WeaponInfoRow)
        {
            bIsFind = true;
            OutInfo = *WeaponInfoRow;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UTopDownShooterGameInstance::GetWeaponInfoByName - WeaponTable - NULL"));
    }

    return bIsFind;
}

bool UTopDownShooterGameInstance::GetDropItemInfoByName(FName NameItem, FDropItem& OutInfo)
{
    bool bIsFound = false;
    
    if (DropItemInfoTable)
    {
        FDropItem* DropItemInfoRow;
        TArray<FName> RowNames = DropItemInfoTable->GetRowNames();
        int8 i = 0;

        while (i < RowNames.Num() && !bIsFound)
        {
            DropItemInfoRow = DropItemInfoTable->FindRow<FDropItem>(RowNames[i], "");
            if (DropItemInfoRow->WeaponInfo.NameItem == NameItem)
            {
                OutInfo = *DropItemInfoRow;
                bIsFound = true;
            }
            i++;
        }
    }

    return bIsFound;
}
