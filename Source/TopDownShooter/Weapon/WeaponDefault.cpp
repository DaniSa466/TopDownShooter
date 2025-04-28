// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponDefault.h"

// Sets default values
AWeaponDefault::AWeaponDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponDefault::BeginPlay()
{
	Super::BeginPlay();

	WeaponInit();
}

// Called every frame
void AWeaponDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireTick(DeltaTime);
	ReloadTick(DeltaTime);
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	if (WeaponFiring)
		if (FireTime < 0.f)
			if (GetWeaponRound() > 0)
				if (!WeaponReloading)
					Fire();

			else
				if (!WeaponReloading)
					InitReload();

		else
			FireTime -= DeltaTime;
}

void AWeaponDefault::ReloadTick(float DeltaTime)
{
	if (WeaponReloading)
	{
		if (ReloadTimer < 0)
			FinishReload();
		else
			ReloadTimer -= DeltaTime;
	}
}

void AWeaponDefault::WeaponInit()
{
	if (SkeletalMeshWeapon && !SkeletalMeshWeapon->SkeletalMesh)
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent();
	}
}

void AWeaponDefault::SetWeaponStateFire(bool bIsFire)
{
	if (CheckWeaponCanFire())
		WeaponFiring = bIsFire;
	else
		WeaponFiring = false;
}

bool AWeaponDefault::CheckWeaponCanFire()
{
	return true;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	FireTime = WeaponSetting.RateOfFire;
	WeaponInfo.Round--;

	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		if (ProjectileInfo.Projectile)
		{
			//Projectile Init ballistic fire

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			AProjectileDefault* myProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (myProjectile)
			{
				myProjectile->InitialLifeSpan = 20.0f;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile hasn't been initialized."));
			//ToDo Projectile null Init trace fire			
		}
	}
}

void AWeaponDefault::UpdateStateWeapon(EMovementState NewMovementState)
{
	//ToDo Dispersion
	ChangeDispersion();
}

void AWeaponDefault::ChangeDispersion()
{
}

int32 AWeaponDefault::GetWeaponRound()
{
	return WeaponInfo.Round;
}

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;

	ReloadTimer = WeaponSetting.ReloadTime;
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;

	WeaponInfo.Round = WeaponSetting.MaxRound;
}
