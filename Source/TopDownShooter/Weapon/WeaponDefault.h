// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "TopDownShooter/FuncLibrary/Types.h"
#include "TopDownShooter/Weapon/ProjectileDefault.h"
#include "WeaponDefault.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFire, UAnimMontage*, Anim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadStart, UAnimMontage*, Anim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadEnd, bool, bIsSuccess, int32, AmmoReamain);

UCLASS()
class TOPDOWNSHOOTER_API AWeaponDefault : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponDefault();
		
	FOnWeaponFire OnWeaponFire;
	FOnWeaponReloadStart OnWeaponReloadStart;
	FOnWeaponReloadEnd OnWeaponReloadEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootLocation = nullptr;

	UPROPERTY()
	FWeaponInfo WeaponSettings;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	FAdditionalWeaponInfo AdditionalWeaponInfo;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Tick func
	virtual void Tick(float DeltaTime) override;

	void FireTick(float DeltaTime);

	void ReloadTick(float DeltaTime);

	void DispersionTick(float DeltaTime);

	void ClipDropTick(float DeltaTime);

	void ShellDropTick(float DeltaTime);

	void WeaponInit();

	FName CurrentWeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
	bool WeaponFiring = false;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	bool WeaponReloading = false;
	bool WeaponAiming = false;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetWeaponStateFire_OnServer(bool bIsFire);

	bool CheckWeaponCanFire();

	FProjectileInfo GetProjectile();

	void Fire();

	UFUNCTION(Server, Reliable)
	void UpdateStateWeapon_OnServer(EMovementState NewMovementState);
	void ChangeDispersionByShoot();
	float GetCurrentDispersion() const;
	FVector ApplyDispersionToShoot(FVector DirectionShoot) const;

	FVector GetFireEndLocation() const;
	int8 GetNumberProjectileByShoot() const;

	//Timers'flags
	float FireTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	float ReloadTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic Debug")	//Remove !!! Debug
	float ReloadTime = 0.0f;

	UFUNCTION(BlueprintCallable)
	int32 GetWeaponRound();

	void InitReload();
	void FinishReload();
	void CancelReload();

	bool CheckWeaponCanBeReloaded();
	int16 GetAvialableAmmo();

	UFUNCTION(BlueprintNativeEvent)
	void InitReload_BP();
	UFUNCTION(BlueprintNativeEvent)
	void FinishReload_BP();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool BlockFire = false;

	//Dispersion
	bool ShouldReduceDispersion = false;
	float CurrentDispersion = 0.f;
	float CurrentDispersionMax = 1.f;
	float CurrentDispersionMin = 0.1f;
	float CurrentDispersionRecoil = 0.1f;
	float CurrentDispersionReduction = 0.1f;

	TArray<FVector> tracesEndLoc;
	
	//may be should change logic and don't use so many arrays
	TArray<FVector> hitImpactPoints;
	TArray<FVector> hitImpactNormals;
	TArray<UPrimitiveComponent*> hitComponents;
	TArray<UMaterialInterface*> hitDecals;
	TArray<UParticleSystem*> hitParticles;

	UPROPERTY(Replicated)
	FVector ShootEndLocation = FVector(0);

	//Drop Meshes
	bool DropClipFlag = false;
	float DropClipTimer = -1.f;
	bool DropShellFlag = false;
	float DropShellTimer = -1.f;

	UFUNCTION(Server, Reliable)
	void InitDropMesh_OnServer(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection,
		float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpuls, float CustomMass);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float SizeVectorToChangeShootDirectionLogic = 100.f;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//void AWeaponDefault::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(Server, Unreliable)
	void UpdateWeaponByCharacterMovementState_OnServer(FVector newShootEndLocation, bool newShouldReduceDispersion);

	UFUNCTION(NetMulticast, Unreliable)
	void WeaponAnimationStart_Multicast(UAnimMontage* newAnim);

	UFUNCTION(NetMulticast, Unreliable)
	void ShellDropFire_Multicast(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection,
		float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpuls, float CustomMass, FVector localDirection);

	UFUNCTION(NetMulticast, Unreliable)
	void SoundAndFXWeaponFire_Multicast(UParticleSystem* fireFX, USoundBase* fireSound);

	UFUNCTION(Server, Reliable)
	void InitTrace_OnServer(FVector spawnLocation, FVector endLocation, bool callMulticastFunc);
	UFUNCTION(NetMulticast, Unreliable)
	void InitTrace_Multicast(FVector_NetQuantize spawnLocation, const TArray<FVector>& endLocations);
	UFUNCTION(NetMulticast, Unreliable)
	void InitEffectsByTraceHit_Multicast(const TArray<FVector>& impactPoints,
		const TArray<FVector>& impactNormals, const TArray<UPrimitiveComponent*>& components, 
		const TArray<UMaterialInterface*>& decals, const TArray<UParticleSystem*>& particles, USoundBase* sound);
};