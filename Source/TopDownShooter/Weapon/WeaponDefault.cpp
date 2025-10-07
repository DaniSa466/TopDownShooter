// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponDefault.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "TopDownShooter/Character/InventoryComponent.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"

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
	ClipDropTick(DeltaTime);
	ShellDropTick(DeltaTime);
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	if (WeaponFiring)
	{
		if (FireTime <= 0.f)
			Fire();

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

void AWeaponDefault::ClipDropTick(float DeltaTime)
{
	if (DropClipFlag)
		if (DropClipTimer < 0.0f)
		{
			DropClipFlag = false;
			InitDropMesh(WeaponSettings.ClipDropMesh.DropMesh, WeaponSettings.ClipDropMesh.DropMeshOffset,
				WeaponSettings.ClipDropMesh.DropMeshImpulsDirection, WeaponSettings.ClipDropMesh.DropMeshLifeTime,
				WeaponSettings.ClipDropMesh.ImpulsRandomDispersion, WeaponSettings.ClipDropMesh.PowerImpuls,
				WeaponSettings.ClipDropMesh.CustomMass);
		}
		else
			DropClipTimer -= DeltaTime;
}

void AWeaponDefault::ShellDropTick(float DeltaTime)
{
	if (DropShellFlag)
		if (DropShellTimer <= 0.0f)
		{
			DropShellFlag = false;
			InitDropMesh(WeaponSettings.ShellDropMesh.DropMesh, WeaponSettings.ShellDropMesh.DropMeshOffset,
				WeaponSettings.ShellDropMesh.DropMeshImpulsDirection, WeaponSettings.ShellDropMesh.DropMeshLifeTime,
				WeaponSettings.ShellDropMesh.ImpulsRandomDispersion, WeaponSettings.ShellDropMesh.PowerImpuls,
				WeaponSettings.ShellDropMesh.CustomMass);
		}
		else
			DropShellTimer -= DeltaTime;
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
	return !BlockFire && !WeaponReloading;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSettings.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	if (GetWeaponRound() > 0)
	{
		// may be will be changed
		UAnimMontage* AnimToPlay = nullptr;
		if (WeaponAiming)
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharAimFire;
		else
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharStandFire;

		if (WeaponSettings.AnimWeaponInfo.AnimWeaponFire && SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
			SkeletalMeshWeapon->GetAnimInstance()->Montage_Play(WeaponSettings.AnimWeaponInfo.AnimWeaponFire);

		if (WeaponSettings.ShellDropMesh.DropMesh)
			if (WeaponSettings.ShellDropMesh.DropMeshTime < 0.0f)
				InitDropMesh(WeaponSettings.ShellDropMesh.DropMesh, WeaponSettings.ShellDropMesh.DropMeshOffset,
					WeaponSettings.ShellDropMesh.DropMeshImpulsDirection, WeaponSettings.ShellDropMesh.DropMeshLifeTime,
					WeaponSettings.ShellDropMesh.ImpulsRandomDispersion, WeaponSettings.ShellDropMesh.PowerImpuls,
					WeaponSettings.ShellDropMesh.CustomMass);
			else
			{
				DropShellFlag = true;
				DropShellTimer = WeaponSettings.ShellDropMesh.DropMeshTime;
			}

		FireTime = WeaponSettings.RateOfFire;
		AdditionalWeaponInfo.Round--;
		ChangeDispersionByShoot();

		//if (AnimToPlay)
		OnWeaponFire.Broadcast(AnimToPlay);

		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSettings.SoundFireWeapon, ShootLocation->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSettings.EffectFireWeapon, ShootLocation->GetComponentTransform());

		int8 NumberProjectile = GetNumberProjectileByShoot();

		if (ShootLocation)
		{
			FVector SpawnLocation = ShootLocation->GetComponentLocation();
			FRotator SpawnRotation;
			FProjectileInfo ProjectileInfo = GetProjectile();

			FVector EndLocation;
			for (int8 i = 0; i < NumberProjectile; i++)//ShootGun
			{
				EndLocation = GetFireEndLocation();

				if (ProjectileInfo.Projectile)
				{
					//Projectile Init ballistic fire
					FVector Dir = EndLocation - SpawnLocation;
					Dir.Normalize();

					FMatrix myMatrix(Dir, FVector(0, 0, 0), FVector(0, 0, 0), FVector::ZeroVector);
					SpawnRotation = myMatrix.Rotator();

					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnParams.Owner = GetOwner();
					SpawnParams.Instigator = GetInstigator();

					AProjectileDefault* myProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, 
						&SpawnLocation, &SpawnRotation, SpawnParams));
					if (myProjectile)
					{
						myProjectile->InitialLifeSpan = 20.0f;
						myProjectile->InitProjectile(WeaponSettings.ProjectileSetting);
					}
				}
				else
				{
					FHitResult Hit;
					TArray<AActor*> Actors;

					UKismetSystemLibrary::LineTraceSingle(GetWorld(), SpawnLocation,
						EndLocation * WeaponSettings.DistanceTrace, ETraceTypeQuery::TraceTypeQuery4,
						false, Actors, EDrawDebugTrace::ForDuration, Hit, true, FLinearColor::Red, 
						FLinearColor::Green, 5.f);

					if (ShowDebug)
						DrawDebugLine(GetWorld(), SpawnLocation, SpawnLocation +
							ShootLocation->GetForwardVector() * WeaponSettings.DistanceTrace, FColor::Black, false,
							5.f, (uint8)'\000', 0.5f);

					if (Hit.GetActor() && Hit.PhysMaterial.IsValid())
					{
						EPhysicalSurface MySurfaceType = UGameplayStatics::GetSurfaceType(Hit);

						if (WeaponSettings.ProjectileSetting.HitDecals.Contains(MySurfaceType))
						{
							UMaterialInterface* myMaterial = WeaponSettings.ProjectileSetting.HitDecals[MySurfaceType];

							if (myMaterial && Hit.GetComponent())
								UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.f),
									Hit.GetComponent(), NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),
									EAttachLocation::KeepWorldPosition, 10.f);
						}

						if (WeaponSettings.ProjectileSetting.HitFXs.Contains(MySurfaceType))
						{
							UParticleSystem* myParticle = WeaponSettings.ProjectileSetting.HitFXs[MySurfaceType];

							if (myParticle)
								UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
									myParticle, FTransform(Hit.ImpactNormal.Rotation(),
										Hit.ImpactPoint, FVector(1.f)));
						}

						if (WeaponSettings.ProjectileSetting.HitSound)
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), 
								WeaponSettings.ProjectileSetting.HitSound, Hit.ImpactPoint);

						UTPS_StatsEffects* NewEffect = NewObject<UTPS_StatsEffects>(Hit.GetActor(), FName("Effect"));

						UGameplayStatics::ApplyPointDamage(Hit.GetActor(),
							WeaponSettings.ProjectileSetting.ProjectileDamage,
							Hit.TraceStart, Hit, GetInstigatorController(), this, NULL);
						/*UGameplayStatics::ApplyDamage(Hit.GetActor(),
							WeaponSettings.ProjectileSetting.ProjectileDamage, 
							GetInstigatorController(), this, NULL);*/
					}
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
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.Stand_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.Stand_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.Stand_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.Stand_StateDispersionReduction;
		WeaponAiming = false;
		break;
	case EMovementState::AimStand_State:
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.AimStand_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.AimStand_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.AimStand_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.AimStand_StateDispersionReduction;
		WeaponAiming = true;
		break;
	case EMovementState::Aim_State:
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.Aim_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.Aim_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.Aim_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.Aim_StateDispersionReduction; 
		WeaponAiming = true;
		break;
	case EMovementState::AimWalk_State:
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.AimWalk_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.AimWalk_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.AimWalk_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.AimWalk_StateDispersionReduction;
		WeaponAiming = true;
		break;
	case EMovementState::Walk_State:
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.Walk_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.Walk_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.Walk_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.Walk_StateDispersionReduction;
		WeaponAiming = false;
		break;
	case EMovementState::Run_State:
		CurrentDispersionMax = WeaponSettings.DispersionWeapon.Run_StateDispersionMax;
		CurrentDispersionMin = WeaponSettings.DispersionWeapon.Run_StateDispersionMin;
		CurrentDispersionRecoil = WeaponSettings.DispersionWeapon.Run_StateDispersionRecoil;
		CurrentDispersionReduction = WeaponSettings.DispersionWeapon.Run_StateDispersionReduction;
		WeaponAiming = false;
		break;
	case EMovementState::SprintRun_State:
		BlockFire = true;
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
	if (tmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		DirectionShoot = ShootLocation->GetComponentLocation() - ShootEndLocation;
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot((DirectionShoot).GetSafeNormal()) * -20000.f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), -DirectionShoot, 
				WeaponSettings.DistanceTrace, GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f, 
				32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}
	else
	{
		DirectionShoot = ShootLocation->GetForwardVector();
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(DirectionShoot) * 20000.f;
		if (ShowDebug)
		DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(),
			DirectionShoot, WeaponSettings.DistanceTrace,
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
	return WeaponSettings.NumProjectileByShoot;
}

int32 AWeaponDefault::GetWeaponRound()
{
	return AdditionalWeaponInfo.Round;
}

void AWeaponDefault::InitReload()
{
	if (GetAvialableAmmo() > 0)
	{
		WeaponReloading = true;

		ReloadTimer = WeaponSettings.ReloadTime;

		UAnimMontage* AnimToPlay = nullptr;
		if (WeaponAiming)
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharAimReload;
		else
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharStandReload;
		if (AnimToPlay)
			OnWeaponReloadStart.Broadcast(AnimToPlay);

		UAnimMontage* AnimWeaponToPlay = nullptr;
		if (WeaponAiming)
			AnimWeaponToPlay = WeaponSettings.AnimWeaponInfo.AnimWeaponAimReload;
		else
			AnimWeaponToPlay = WeaponSettings.AnimWeaponInfo.AnimWeaponReload;

		if (AnimWeaponToPlay && SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
			SkeletalMeshWeapon->GetAnimInstance()->Montage_Play(AnimWeaponToPlay);

		if (WeaponSettings.ClipDropMesh.DropMesh)
		{
			DropClipFlag = true;
			DropClipTimer = WeaponSettings.ClipDropMesh.DropMeshTime;
		}

		InitReload_BP();
	}
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;
	int8 AvialableAmmo;

	if (GetAvialableAmmo() > WeaponSettings.MaxRound)
		AvialableAmmo = WeaponSettings.MaxRound;
	else
		AvialableAmmo = GetAvialableAmmo();

	int8 AmmoTaken = WeaponSettings.MaxRound - AdditionalWeaponInfo.Round;

	if (AmmoTaken > AvialableAmmo)
		AdditionalWeaponInfo.Round += AvialableAmmo;
	else
		AdditionalWeaponInfo.Round += AmmoTaken;

	if (AmmoTaken > AvialableAmmo)
		AmmoTaken = AvialableAmmo;
	OnWeaponReloadEnd.Broadcast(true, -AmmoTaken);
	FinishReload_BP();
}

void AWeaponDefault::CancelReload()
{
	WeaponReloading = false;

	if (SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
		SkeletalMeshWeapon->GetAnimInstance()->StopAllMontages(0.15f);

	OnWeaponReloadEnd.Broadcast(false, 0);
	DropClipFlag = false;
}


int16 AWeaponDefault::GetAvialableAmmo()
{
	int16 AvialableAmmoForWeapon = WeaponSettings.MaxRound;
	if (GetOwner())
	{
		UInventoryComponent* MyInventory = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (MyInventory)
		{
			MyInventory->CheckAmmoForWeapon(WeaponSettings.WeaponType, AvialableAmmoForWeapon);	
		}
	}
	return AvialableAmmoForWeapon;
}


void AWeaponDefault::InitReload_BP_Implementation()
{
	//In BluePrints
}

void AWeaponDefault::FinishReload_BP_Implementation()
{
	//In BluePrints
}

void AWeaponDefault::InitDropMesh(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection, 
	float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass)
{
	if (DropMesh)
	{

		FTransform Transform;

		FVector LocalDirection = this->GetActorForwardVector() * Offset.GetLocation().X + this->GetActorRightVector() * Offset.GetLocation().Y +
			this->GetActorUpVector() * Offset.GetLocation().Z;

		Transform.SetLocation(GetActorLocation() + LocalDirection);
		Transform.SetScale3D(Offset.GetScale3D());
		Transform.SetRotation((GetActorRotation() + Offset.Rotator()).Quaternion());

		AStaticMeshActor* ActorToSpawn = nullptr;
		FActorSpawnParameters Params;

		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Params.Owner = this;
		ActorToSpawn = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform, Params);

		if (ActorToSpawn && ActorToSpawn->GetStaticMeshComponent())
		{
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("IgnoreOnlyPawn"));
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			ActorToSpawn->GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
			ActorToSpawn->GetStaticMeshComponent()->SetSimulatePhysics(true);
			ActorToSpawn->GetStaticMeshComponent()->SetStaticMesh(DropMesh);

			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECollisionResponse::ECR_Block);
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECollisionResponse::ECR_Block);
			ActorToSpawn->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECollisionResponse::ECR_Block);

			ActorToSpawn->SetActorTickEnabled(false);
			ActorToSpawn->InitialLifeSpan = LifeTimeMesh;

			if (CustomMass > 0.0f)
				ActorToSpawn->GetStaticMeshComponent()->SetMassOverrideInKg(NAME_None, CustomMass, true);

			if (!DropImpulseDirection.IsNearlyZero())
			{
				FVector FinalDirection;
				LocalDirection += DropImpulseDirection * 1000.f;

				if (!FMath::IsNearlyZero(ImpulseRandomDispersion))
					FinalDirection += UKismetMathLibrary::RandomUnitVectorInConeInDegrees(LocalDirection, ImpulseRandomDispersion);
				FinalDirection.GetSafeNormal(0.0001f);

				ActorToSpawn->GetStaticMeshComponent()->AddImpulse(FinalDirection * PowerImpulse);
			}
		}
	}
}

