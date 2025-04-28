// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "TopDownShooter/Weapon/WeaponDefault.h"
#include "Engine/DataTable.h"
#include "TopDownShooterGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TOPDOWNSHOOTER_API UTopDownShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSettng")
	UDataTable* WeaponInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
	bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo);
};
