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

bool UTopDownShooterGameInstance::GetDropItemInfoByWeaponName(FName NameItem, FDropItem& OutInfo)
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

    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGameInstance::GetWeaponItemInfoByWeaponName - DropItemInfoTable is NULL"));
    }

    return bIsFound;
}

bool UTopDownShooterGameInstance::GetDropItemInfoByName(FName NameItem, FDropItem& OutInfo)
{
    bool bIsFound = false;
    FDropItem* DropItemInfoRow;

    if (DropItemInfoTable)
    {
        DropItemInfoRow = DropItemInfoTable->FindRow<FDropItem>(NameItem, "", false);

        if (DropItemInfoRow)
        {
            OutInfo = *DropItemInfoRow;
            bIsFound = true;
        }
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("UGameInstance::GetDropItemInfoByName - DropItemInfoTable - NULL"));

    return bIsFound;
}
