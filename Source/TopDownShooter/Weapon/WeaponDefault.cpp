// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponDefault.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

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
	DispersionTick(DeltaTime);
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	if (WeaponFiring)
	{
		if (FireTime <= 0.f)
		{
			if (!WeaponReloading)
				Fire();
		}

		else
			FireTime -= DeltaTime;
	}
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

void AWeaponDefault::DispersionTick(float DeltaTime)
{
	if (!WeaponReloading)
	{
		if (!WeaponFiring)
		{
			if (ShouldReduceDispersion)
				CurrentDispersion -= CurrentDispersionReduction;
			else
				CurrentDispersion += CurrentDispersionReduction;
		}

		if (CurrentDispersion < CurrentDispersionMin)
			CurrentDispersion = CurrentDispersionMin;

		else
			if (CurrentDispersion > CurrentDispersionMax)
				CurrentDispersion = CurrentDispersionMax;
	}

	else
	{
		CurrentDispersion -= CurrentDispersionReduction;

		if (CurrentDispersion < CurrentDispersionMin)
			CurrentDispersion = CurrentDispersionMin;
	}

	if (ShowDebug)
		UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f."), CurrentDispersionMax, CurrentDispersionMin, CurrentDispersion);
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
	{
		WeaponFiring = false;
		FireTime = 0.01f;
	}
}

bool AWeaponDefault::CheckWeaponCanFire()
{
	return !BlockFire;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	if (GetWeaponRound() > 0)
	{

		FireTime = WeaponSetting.RateOfFire;
		WeaponInfo.Round--;
		ChangeDispersionByShoot();

		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());

		int8 NumberProjectile = GetNumberProjectileByShoot();

		if (ShootLocation)
		{
			FVector SpawnLocation = ShootLocation->GetComponentLocation();
			FRotator SpawnRotation;
			FProjectileInfo ProjectileInfo;
			ProjectileInfo = GetProjectile();

			FVector EndLocation;
			for (int8 i = 0; i < NumberProjectile; i++)//ShootGun
			{
				EndLocation = GetFireEndLocation();

				FVector Dir = EndLocation - SpawnLocation;
				Dir.Normalize();

				FMatrix myMatrix(Dir, FVector(0, 0, 0), FVector(0, 0, 0), FVector::ZeroVector);
				SpawnRotation = myMatrix.Rotator();

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
						myProjectile->InitProjectile(WeaponSetting.ProjectileSetting);
					}
				}
				else
				{
					//ToDo Projectile null Init trace fire			
				}
			}
		}
	}
	else
		if (!WeaponReloading)
			InitReload();
}

void AWeaponDefault::UpdateStateWeapon(EMovementState NewMovementState)
{
	//ToDo Dispersion	
	BlockFire = false;

	switch (NewMovementState)
	{
	case EMovementState::Stand_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Stand_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Stand_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Stand_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Stand_StateDispersionReduction;
		break;
	case EMovementState::AimStand_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimStand_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimStand_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimStand_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.AimStand_StateDispersionReduction;
		break;
	case EMovementState::Aim_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Aim_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Aim_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Aim_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction; 
		break;
	case EMovementState::AimWalk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionReduction;
		break;
	case EMovementState::Walk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Walk_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Walk_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Walk_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Walk_StateDispersionReduction;
		break;
	case EMovementState::Run_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Run_StateDispersionMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Run_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Run_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Run_StateDispersionReduction;
		break;
	case EMovementState::SprintRun_State:
		BlockFire = true;
		SetWeaponStateFire(false);
		break;
	default:
		break;
	}
}

void AWeaponDefault::ChangeDispersionByShoot()
{
	CurrentDispersion += CurrentDispersionRecoil;
}

float AWeaponDefault::GetCurrentDispersion() const
{
	return CurrentDispersion;
}

FVector AWeaponDefault::ApplyDispersionToShoot(FVector DirectionShoot) const
{
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.f);
}

FVector AWeaponDefault::GetFireEndLocation() const
{
	bool bShootDirection = false;
	FVector EndLocation = FVector(0);
	FVector DirectionShoot;

	
	FVector tmpV = (ShootLocation->GetComponentLocation() - ShootEndLocation);
	//UE_LOG(LogTemp, Warning, TEXT("Vector X = %f. Y = %f. Size = %f"), tmpV.X, tmpV.Y, tmpV.Size());
	if (tmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		DirectionShoot = ShootLocation->GetComponentLocation() - ShootEndLocation;
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot((DirectionShoot).GetSafeNormal()) * -20000.f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), -DirectionShoot, 
				WeaponSetting.DistanceTrace, GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f, 
				32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}
	else
	{
		DirectionShoot = ShootLocation->GetForwardVector();
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(DirectionShoot) * 20000.f;
		if (ShowDebug)
		DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(),
			DirectionShoot, WeaponSetting.DistanceTrace,
			GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f,
			32, FColor::Emerald, false, .1f, (uint8)'\000', 1.f);
	}

	if (ShowDebug)
	{
		//direction weapon look
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(),
			ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * 500.f, FColor::Cyan,
			false, 5.f, (uint8)'\000', 0.5f);
		//direction projectile must fly
		DrawDebugLine(GetWorld(),ShootLocation->GetComponentLocation(), ShootEndLocation, 
			FColor::Red, false, 5.f, (uint8)'\000', 0.5f);
		//diretcion projectile current fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndLocation, 
			FColor::Black, false, 5.f, (uint8)'\000', 0.5f);

		//DrawDebugSphere(GetWorld(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector()*SizeVectorToChangeShootDirectionLogic, 10.f, 8, FColor::Red, false, 4.0f);
	}

	return EndLocation;
}

int8 AWeaponDefault::GetNumberProjectileByShoot() const
{
	return WeaponSetting.NumProjectileByShoot;
}

int32 AWeaponDefault::GetWeaponRound()
{
	return WeaponInfo.Round;
}

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;

	ReloadTimer = WeaponSetting.ReloadTime;

	if(WeaponSetting.AnimCharReload)
		OnWeaponReloadStart.Broadcast(WeaponSetting.AnimCharReload);
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;

	WeaponInfo.Round = WeaponSetting.MaxRound;

	OnWeaponReloadEnd.Broadcast();
}
