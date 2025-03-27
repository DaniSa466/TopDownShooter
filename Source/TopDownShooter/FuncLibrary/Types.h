// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Aim_State UMETA(DisplayName = "Aim State"), 
	Walk_State UMETA(DisplayName = "Walk State"),
	AimWalk_State UMETA(DisplayName = "AinWalk_State"),
	SprintRun_State UMETA(DisplayName = "SprintRun_State"),
	Run_State UMETA(DisplayName = "Run State")
};

USTRUCT(BlueprintType)
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Aim_Speed = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Walk_Speed = 400;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Run_Speed = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AimWalk_State = 125;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintRun_State = 800;
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TSubclassOf<class AProjectileDefault> Projectile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileDamage = 20;
};

USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float WeaponDamage = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	float RateOfFire = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
	FProjectileInfo ProjectileSetting;
};

UCLASS()
class TOPDOWNSHOOTER_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};