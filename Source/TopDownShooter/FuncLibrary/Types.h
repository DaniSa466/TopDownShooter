// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileLifeTime = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileInitSpeed = 2000;

	//bomb
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	bool bIsLikeBomb = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileMaxRadiusDamage = 200;
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float DispersionAimStart = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float DispersionAimMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float DispersionAimMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float DispersionAimShootCoef = 1.0f;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class AWeaponDefault> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSetting")
		float RateOfFire = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float ReloadTime = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 MaxRound = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	FWeaponDispersion DispersionWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundFireWepaon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fx")
	UParticleSystem* EffectFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FProjectileInfo ProjectileSetting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float WeaponDamage = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float DistanceTrace = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect")
	UDecalComponent* DecalOnHit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* AnimCharFire = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* AnimCharReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* MagazineDrop = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* SleeveBullets = nullptr;
};

USTRUCT(BlueprintType)
struct FAdditionalWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 Round = 10;
};

UCLASS()
class TOPDOWNSHOOTER_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};