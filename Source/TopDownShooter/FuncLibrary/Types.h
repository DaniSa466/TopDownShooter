// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Stand_State UMETA(DisplayName = "Stand_State"),
	AimStand_State UMETA(DisplatName = "AimStand_State"),
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
	float AimWalk_Speed = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Aim_Speed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Walk_Speed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Run_Speed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintRun_Speed = 800.f;
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TSubclassOf<class AProjectileDefault> Projectile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileLifeTime = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileInitSpeed = 2000.f;

	//bomb
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	bool bIsLikeBomb = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileMaxRadiusDamage = 200.f;
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()

	//Dispersion while Standing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float Stand_StateDispersionMax = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Stand_StateDispersionMin = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Stand_StateDispersionRecoil = 0.4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Stand_StateDispersionReduction = 0.2f;

	//Dispersion While Standing and Aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimStand_StateDispersionMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimStand_StateDispersionMin = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimStand_StateDispersionRecoil = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimStand_StateDispersionReduction = 0.3f;

	//Dispersion while Aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionMax = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionMin = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionRecoil = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionReduction = 0.3f;

	//Dispersion while Walking and Aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionMax = 1.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionMin = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionRecoil = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionReduction = 0.2f;

	//Dispersion while Walking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionMax = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionMin = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionReduction = 0.2f;

	//Dispersion while Running
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionMax = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionMin = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionReduction = 0.1f;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class AWeaponDefault> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float RateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float ReloadTime = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 MaxRound = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 NumProjectileByShoot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	FWeaponDispersion DispersionWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fx")
	UParticleSystem* EffectFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FProjectileInfo ProjectileSetting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float WeaponDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float DistanceTrace = 2000.f;

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
	int32 Round = 45;
};

UCLASS()
class TOPDOWNSHOOTER_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};