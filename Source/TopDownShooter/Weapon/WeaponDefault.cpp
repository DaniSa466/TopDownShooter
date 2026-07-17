// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponDefault.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "TopDownShooter/Character/TopDownShooterCharacter.h"
#include "TopDownShooter/Character/InventoryComponent.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "Net/UnrealNetwork.h"

int32 debugWeaponShow = 0;
FAutoConsoleVariableRef CVarWeaponShow(
	TEXT("TPS.DebugWeapon"), debugWeaponShow,
	TEXT("Draw Debug for Weapon"), ECVF_Cheat);

// Sets default values
AWeaponDefault::AWeaponDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

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

	if (HasAuthority())
	{
		FireTick(DeltaTime);
		ReloadTick(DeltaTime);
		DispersionTick(DeltaTime);
		ClipDropTick(DeltaTime);
		ShellDropTick(DeltaTime);
	}
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
}

void AWeaponDefault::ClipDropTick(float DeltaTime)
{
	if (DropClipFlag)
		if (DropClipTimer < 0.0f)
		{
			DropClipFlag = false;
			InitDropMesh_OnServer(WeaponSettings.ClipDropMesh.DropMesh, WeaponSettings.ClipDropMesh.DropMeshOffset,
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
			InitDropMesh_OnServer(WeaponSettings.ShellDropMesh.DropMesh, WeaponSettings.ShellDropMesh.DropMeshOffset,
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

void AWeaponDefault::SetWeaponStateFire_OnServer_Implementation(bool bIsFire)
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
	//On Server by weapon fire bool

	if (GetWeaponRound() > 0)
	{
		// may be will be changed
		UAnimMontage* AnimToPlay = nullptr;
		if (WeaponAiming)
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharAimFire;
		else
			AnimToPlay = WeaponSettings.AnimWeaponInfo.AnimCharStandFire;

		if (WeaponSettings.AnimWeaponInfo.AnimWeaponFire)
			WeaponAnimationStart_Multicast(WeaponSettings.AnimWeaponInfo.AnimWeaponFire);

		if (WeaponSettings.ShellDropMesh.DropMesh)
			if (WeaponSettings.ShellDropMesh.DropMeshTime < 0.0f)
				InitDropMesh_OnServer(WeaponSettings.ShellDropMesh.DropMesh, WeaponSettings.ShellDropMesh.DropMeshOffset,
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

		SoundAndFXWeaponFire_Multicast(WeaponSettings.EffectFireWeapon, WeaponSettings.SoundFireWeapon);

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
				bool shotByProjectile = false;
				bool callInitTraceMulticast = false;

				if (ProjectileInfo.Projectile)
				{
					//Projectile Init ballistic fire
					FVector Dir = EndLocation - SpawnLocation;
					Dir.Normalize();

					FMatrix myMatrix(Dir, FVector(0, 0, 0), FVector(0, 0, 0), FVector::ZeroVector);
					SpawnRotation = myMatrix.Rotator();
					//SpawnRotation = Dir.Rotation();

					APawn* ownerPawn = Cast<APawn>(GetOwner());

					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnParams.Owner = ownerPawn;
					SpawnParams.Instigator = ownerPawn;

					AProjectileDefault* myProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, 
						&SpawnLocation, &SpawnRotation, SpawnParams));
					if (myProjectile)
					{
						myProjectile->InitialLifeSpan = 20.0f;
						shotByProjectile = myProjectile->InitProjectile(WeaponSettings.ProjectileSetting);
					}

					if (!shotByProjectile)
					{
						myProjectile->Destroy();
						if (tracesEndLoc.Num() == NumberProjectile - 1)
							callInitTraceMulticast = true;

						InitTrace_OnServer(SpawnLocation, EndLocation, callInitTraceMulticast);
					}
				}
				else
				{
					//Shoot with trace
					if (tracesEndLoc.Num() == NumberProjectile - 1)
						callInitTraceMulticast = true;

					InitTrace_OnServer(SpawnLocation, EndLocation, callInitTraceMulticast);
				}
			}
		}
	}
	else
		if (!WeaponReloading && CheckWeaponCanBeReloaded())
			InitReload();
}

void AWeaponDefault::UpdateStateWeapon_OnServer_Implementation(EMovementState NewMovementState)
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
		else
			if (ATopDownShooterCharacter* pointerToCharacter = Cast<ATopDownShooterCharacter>(GetOwner()))
			{
				pointerToCharacter->WeaponReloadStart(AnimToPlay);
				pointerToCharacter = nullptr;
			}

		UAnimMontage* AnimWeaponToPlay = nullptr;
		if (WeaponAiming)
			AnimWeaponToPlay = WeaponSettings.AnimWeaponInfo.AnimWeaponAimReload;
		else
			AnimWeaponToPlay = WeaponSettings.AnimWeaponInfo.AnimWeaponReload;

		if (AnimWeaponToPlay && SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
			WeaponAnimationStart_Multicast(AnimWeaponToPlay);
		//SkeletalMeshWeapon->GetAnimInstance()->Montage_Play(AnimWeaponToPlay); del

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


bool AWeaponDefault::CheckWeaponCanBeReloaded()
{
	bool result = true;
	if (GetOwner())
	{
		UInventoryComponent* myInventory = Cast<UInventoryComponent>(GetOwner()->GetComponentByClass(UInventoryComponent::StaticClass()));
		if (myInventory)
		{
			int16 avialableAmmoForWeapon;
			if (!myInventory->CheckAmmoForWeapon(WeaponSettings.WeaponType, avialableAmmoForWeapon))
			{
				result = false;
				myInventory->WeaponHasNoRoundEvent_Multicast(myInventory->GetWeaponIndexSlotByName(CurrentWeaponName));
			}
			else
			{
				myInventory->WeaponHasRoundEvent_Multicast(myInventory->GetWeaponIndexSlotByName(CurrentWeaponName));
			}
		}
	}

	return result;
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

void AWeaponDefault::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponDefault, AdditionalWeaponInfo);
	DOREPLIFETIME(AWeaponDefault, WeaponReloading);
	DOREPLIFETIME(AWeaponDefault, ShootEndLocation);

}

void AWeaponDefault::InitDropMesh_OnServer_Implementation(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection,
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
	
		ShellDropFire_Multicast(DropMesh, Transform, DropImpulseDirection, LifeTimeMesh,
			ImpulseRandomDispersion, PowerImpulse, CustomMass, LocalDirection);
	}
}

void AWeaponDefault::UpdateWeaponByCharacterMovementState_OnServer_Implementation(FVector newShootEndLocation, bool newShouldReduceDispersion)
{
	ShootEndLocation = newShootEndLocation;
	ShouldReduceDispersion = newShouldReduceDispersion;
}

void AWeaponDefault::WeaponAnimationStart_Multicast_Implementation(UAnimMontage* newFireAnim)
{
	if (newFireAnim && SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
		SkeletalMeshWeapon->GetAnimInstance()->Montage_Play(newFireAnim);
}

void AWeaponDefault::ShellDropFire_Multicast_Implementation(UStaticMesh* DropMesh, FTransform Offset, FVector DropImpulseDirection,
	float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpuls, float CustomMass, FVector localDirection)
{
	FActorSpawnParameters Params;

	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	AStaticMeshActor* ActorToSpawn = nullptr;	
	ActorToSpawn = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Offset, Params);

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
			localDirection += DropImpulseDirection * 1000.f;

			if (!FMath::IsNearlyZero(ImpulseRandomDispersion))
				FinalDirection += UKismetMathLibrary::RandomUnitVectorInConeInDegrees(localDirection, ImpulseRandomDispersion);
			FinalDirection.GetSafeNormal(0.0001f);

			ActorToSpawn->GetStaticMeshComponent()->AddImpulse(FinalDirection * PowerImpuls);
		}
	}
}

void AWeaponDefault::SoundAndFXWeaponFire_Multicast_Implementation(UParticleSystem* fireFX, USoundBase* fireSound)
{
	if (fireFX)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), fireFX, ShootLocation->GetComponentTransform());
	if (fireSound)
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), fireSound, ShootLocation->GetComponentLocation());
}

void AWeaponDefault::InitTrace_OnServer_Implementation(FVector spawnLocation, FVector endLocation, 
	bool callMulticastFunc)
{
	FHitResult hit;
	TArray<AActor*> actors;
	const FProjectileInfo& projectileInfo = GetProjectile();
	endLocation *= WeaponSettings.DistanceTrace;

	UKismetSystemLibrary::LineTraceSingle(GetWorld(), spawnLocation,
		endLocation, ETraceTypeQuery::TraceTypeQuery4,
		false, actors, EDrawDebugTrace::ForDuration, hit, true, FLinearColor::Red,
		FLinearColor::Green, 0.f);

	tracesEndLoc.Add(hit.Location);

	if (ShowDebug)
		DrawDebugLine(GetWorld(), spawnLocation, spawnLocation +
			ShootLocation->GetForwardVector() * WeaponSettings.DistanceTrace, FColor::Black, false,
			5.f, (uint8)'\000', 0.5f);

	if (callMulticastFunc)
	{
		InitTrace_Multicast(spawnLocation, tracesEndLoc);
		tracesEndLoc.Empty();
	}

	if (hit.GetActor() && hit.PhysMaterial.IsValid())
	{
		UMaterialInterface* myMaterial = nullptr;
		UParticleSystem* myParticle = nullptr;
		USoundBase* hitSound = nullptr;

		EPhysicalSurface surfaceType = UGameplayStatics::GetSurfaceType(hit);
		if (WeaponSettings.ProjectileSetting.HitDecals.Contains(surfaceType))
		{
			myMaterial = WeaponSettings.ProjectileSetting.HitDecals[surfaceType];

			//if (myMaterial)
			//	UGameplayStatics::SpawnDecalAtLocation(GetWorld(), myMaterial, FVector(20.f), 
			//		impactPoint, impactNormal.Rotation(), 10.f);
		}

		if (WeaponSettings.ProjectileSetting.HitFXs.Contains(surfaceType))
			myParticle = WeaponSettings.ProjectileSetting.HitFXs[surfaceType];

		if (WeaponSettings.ProjectileSetting.HitSound)
			hitSound = WeaponSettings.ProjectileSetting.HitSound;

		hitImpactPoints.Add(hit.ImpactPoint);
		hitImpactNormals.Add(hit.ImpactNormal);
		hitComponents.Add(hit.GetComponent());
		hitDecals.Add(myMaterial);
		hitParticles.Add(myParticle);

		if (callMulticastFunc)
		{
			InitEffectsByTraceHit_Multicast(hitImpactPoints, hitImpactNormals, hitComponents, hitDecals, hitParticles, hitSound);

			hitImpactPoints.Empty();
			hitImpactNormals.Empty();
			hitComponents.Empty();
			hitDecals.Empty();
			hitParticles.Empty();
		}

		UTypes::AddEffectBySurfaceType(hit.GetActor(), hit.BoneName, projectileInfo.Effect, UGameplayStatics::GetSurfaceType(hit));

		UGameplayStatics::ApplyPointDamage(hit.GetActor(),
			WeaponSettings.ProjectileSetting.ProjectileDamage,
			hit.TraceStart, hit, GetInstigatorController(), this, NULL);
	}
}

void AWeaponDefault::InitTrace_Multicast_Implementation(FVector_NetQuantize spawnLocation, 
	const TArray<FVector>& endLocations)
{
	/*if (!HasAuthority())*/
	{
		//UParticleSystemComponent* particle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
		//		trace, spawnLocation, (spawnLocation - endLocation).Rotation(), true);

		//if (particle)
		//	particle->SetVectorParameter("TargetLocation", endLocation);

		FHitResult hit;
		TArray<AActor*> actors;

		for (int8 i = 0; i < endLocations.Num(); i++)
		{
			UKismetSystemLibrary::LineTraceSingle(GetWorld(), spawnLocation,
				endLocations[i], ETraceTypeQuery::TraceTypeQuery4,
				false, actors, EDrawDebugTrace::ForDuration, hit, true, FLinearColor::Red,
				FLinearColor::Green, 5.f);
		}
	}
}

void AWeaponDefault::InitEffectsByTraceHit_Multicast_Implementation(const TArray<FVector>& impactPoints,
	const TArray<FVector>& impactNormals, const TArray<UPrimitiveComponent*>& components,
	const TArray<UMaterialInterface*>& decals, const TArray<UParticleSystem*>& particles, USoundBase* sound)
{
	bool server = HasAuthority();
	UE_LOG(LogTemp, Warning, TEXT("InitEffectByTrace -- server = %s"), server ? TEXT("TRUE") : TEXT("false"));

	for (int32 i = 0; i < impactPoints.Num(); i++)
	{
		if (decals.IsValidIndex(i) && decals[i] && components.IsValidIndex(i) && components[i])
			UGameplayStatics::SpawnDecalAttached(decals[i], FVector(20.f),
				components[i], NAME_None, impactPoints[i], impactNormals[i].Rotation(),
				EAttachLocation::KeepWorldPosition, 10.f);

		if (particles.IsValidIndex(i) && particles[i])
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
				particles[i], FTransform(impactNormals[i].Rotation(),
					impactPoints[i], FVector(1.f)));

		if (sound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(),
				sound, impactPoints[i]);
	}
}
