// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "InventoryComponent.h"
#include "TopDownShooter/Game/TopDownShooterPlayerController.h"
#include "TopDownShooter/StateEffects/TPS_StatsEffects.h"
#include "TPS_CharHealthComponent.h"
#include "TopDownShooter/Game/TopDownShooterGameInstance.h"
#include "TopDownShooter/Weapon/ProjectileDefault.h"
#include "Materials/Material.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"
#include "TopDownShooter/TopDownShooter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

ATopDownShooterCharacter::ATopDownShooterCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	HealthComponent = CreateDefaultSubobject<UTPS_CharHealthComponent>(TEXT("HealthComponent"));

	if (InventoryComponent)
		InventoryComponent->OnSwitchWeapon.AddDynamic(this, &ATopDownShooterCharacter::InitWeapon);

	if (HealthComponent)
		HealthComponent->OnDead.AddDynamic(this, &ATopDownShooterCharacter::CharDead);

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	//NetWork
	bReplicates = true;
}

void ATopDownShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocallyControlled())
	{
		ChangeMovementState();
		StaminaSystem(MovementState);
		SprintDirectionLimitation(MovementState);
		MovementTick(DeltaSeconds);
	}

	if(CurrentCursor)
	{
		APlayerController* myPC = Cast<APlayerController>(GetController());
		if (myPC && myPC->IsLocalPlayerController())
		{
			FHitResult TraceHitResult;
			myPC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
		}
	}
}

void ATopDownShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		if (CursorMaterial && (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority))
		{
			CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
		}
	}
}

void ATopDownShooterCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATopDownShooterCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATopDownShooterCharacter::InputAxisY);

	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, 
		this, &ATopDownShooterCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, 
		this, &ATopDownShooterCharacter::InputAttackReleased);
	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, 
		this, &ATopDownShooterCharacter::TryReloadWeapon);

	NewInputComponent->BindAction(TEXT("SwitchNextWeapon"), EInputEvent::IE_Pressed, 
		this, &ATopDownShooterCharacter::SwitchNextWeapon_OnServer);
	NewInputComponent->BindAction(TEXT("SwitchPreviousWeapon"), EInputEvent::IE_Pressed, 
		this, &ATopDownShooterCharacter::SwitchPreviousWeapon_OnServer);

	NewInputComponent->BindAction(TEXT("AbilityAction"), EInputEvent::IE_Pressed,
		this, &ATopDownShooterCharacter::TryAbilityEnabled);

	NewInputComponent->BindAction(TEXT("DropCurrentWeapon"), EInputEvent::IE_Pressed,
		this, &ATopDownShooterCharacter::DropCurrentWeapon);

	TArray<FKey> HotKeys;
	HotKeys.Add(EKeys::One);
	HotKeys.Add(EKeys::Two);
	HotKeys.Add(EKeys::Three);
	HotKeys.Add(EKeys::Four);
	HotKeys.Add(EKeys::Five);
	HotKeys.Add(EKeys::Six);
	HotKeys.Add(EKeys::Seven);
	HotKeys.Add(EKeys::Eight);
	HotKeys.Add(EKeys::Nine);
	HotKeys.Add(EKeys::Zero);

	NewInputComponent->BindKey(HotKeys[1], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<1>);
	NewInputComponent->BindKey(HotKeys[2], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<2>);
	NewInputComponent->BindKey(HotKeys[3], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<3>);
	NewInputComponent->BindKey(HotKeys[4], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<4>);
	NewInputComponent->BindKey(HotKeys[5], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<5>);
	NewInputComponent->BindKey(HotKeys[6], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<6>);
	NewInputComponent->BindKey(HotKeys[7], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<7>);
	NewInputComponent->BindKey(HotKeys[8], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<8>);
	NewInputComponent->BindKey(HotKeys[9], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<9>);
	NewInputComponent->BindKey(HotKeys[0], IE_Pressed, this, &ATopDownShooterCharacter::TKeyPressed<0>);
}

void ATopDownShooterCharacter::SetSpeedCoef(float newCoef)
{
	speedUpCoef = newCoef;
}

void ATopDownShooterCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void ATopDownShooterCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void ATopDownShooterCharacter::InputAttackPressed()
{
	if (IsAlive)
		AttackCharEvent(true);
}

void ATopDownShooterCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void ATopDownShooterCharacter::TryReloadWeapon()
{
	if (IsAlive && CurrentWeapon && !CurrentWeapon->WeaponReloading)
		TryReloadWeapon_OnServer();
}

void ATopDownShooterCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (myWeapon)
		myWeapon->SetWeaponStateFire_OnServer(bIsFiring);

	else
		UE_LOG(LogTemp, Warning, TEXT("ATopDownShooterCharacter::AttackCharEvent - CurrentWeapon - NULL"));
}

void ATopDownShooterCharacter::MovementTick(float DeltaTime)
{
	if (IsAlive)
	{
		if (GetController() && GetController()->IsLocalController())
		{
			AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
			AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

			APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

			if (MyController)
			{
				FHitResult ResultHit;
				MyController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);

				float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;

				SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
				SetActorRotationByYaw_OnServer(FindRotatorResultYaw);

				if (CurrentWeapon)
				{
					FVector Displacement = FVector(0);

					switch (MovementState)
					{
					case EMovementState::Stand_State:
						Displacement = FVector(0.f, 0.f, 120.f);
						break;
					case EMovementState::AimStand_State:
						Displacement = FVector(0.f, 0.f, 160.f);
						break;
					case EMovementState::Aim_State:
						Displacement = FVector(0.f, 0.f, 160.f);
						break;
					case EMovementState::AimWalk_State:
						Displacement = FVector(0.f, 0.f, 160.f);
						break;
					case EMovementState::Walk_State:
						Displacement = FVector(0.f, 0.f, 120.f);
						break;
					case EMovementState::Run_State:
						Displacement = FVector(0.f, 0.f, 120.f);
						break;
					case EMovementState::SprintRun_State:
						break;
					default:
						break;
					}

					bool bIsReducingDispersion;
					if (FMath::IsNearlyZero(GetVelocity().Size(), 0.5f))
						bIsReducingDispersion = true;
						//CurrentWeapon->ShouldReduceDispersion = true; del
					else
						bIsReducingDispersion = false;
						//CurrentWeapon->ShouldReduceDispersion = false; del
					
					CurrentWeapon->UpdateWeaponByCharacterMovementState_OnServer(ResultHit.Location + Displacement, bIsReducingDispersion);
					//CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement; del
				}
			}
		}
	}
}

void ATopDownShooterCharacter::CharacterUpdate()
{
	switch (MovementState)
	{
	case EMovementState::Aim_State:
		ResSpeed = MovementSpeedInfo.Aim_Speed;
		break;
	case EMovementState::Walk_State:
		ResSpeed = MovementSpeedInfo.Walk_Speed;
		break;
	case EMovementState::AimWalk_State:
		ResSpeed = MovementSpeedInfo.AimWalk_Speed;
		break;
	case EMovementState::Run_State:
		ResSpeed = MovementSpeedInfo.Run_Speed;
		break;
	case EMovementState::SprintRun_State:
		ResSpeed = MovementSpeedInfo.SprintRun_Speed;
		break;
	default:
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed * speedUpCoef;
}

void ATopDownShooterCharacter::ChangeMovementState()
{
	EMovementState newState = EMovementState::Run_State;

	if (AxisX != 0 || AxisY != 0)
	{
		if (!WalkEnabled && !SprintRunEnabled && !AimEnabled)
			newState = EMovementState::Run_State;

		else
		{
			if (SprintRunEnabled)
			{
				if (Stamina > 0)
				{
					WalkEnabled = false;
					AimEnabled = false;
					newState = EMovementState::SprintRun_State;
				}
				else
					SprintRunEnabled = false;
			}	

			else if (WalkEnabled && !SprintRunEnabled && AimEnabled)
				newState = EMovementState::AimWalk_State;

			else
			{
				if (WalkEnabled && !SprintRunEnabled && !AimEnabled)
					newState = EMovementState::Walk_State;

				else
					newState = EMovementState::Aim_State;
			}
		}
	}

	else
	{
		if (AimEnabled)
			newState = EMovementState::AimStand_State;
		else
			newState = EMovementState::Stand_State;
	}

	//Weapon state update
	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
		myWeapon->UpdateStateWeapon_OnServer(newState);
}

//Function which controls character's stamina and doesn't let him Sprint for a long time
void ATopDownShooterCharacter::StaminaSystem(EMovementState State)
{
	if (State == EMovementState::SprintRun_State && (AxisX != 0 || AxisY != 0))
	{
		Stamina -= 1;

		// if after decreasing stamina it besomes equal to zero, MovementState changes to Run_State
		if (Stamina <= 0)
		{
			MovementState = EMovementState::Run_State;
			CharacterUpdate();
		}
	}

	else
	{
		// system which doesn't let characters stamina increase forever
		if (Stamina < MaxStamina) Stamina += 1;
	}
}

//Finction which is responsible for sprinting only forward
void ATopDownShooterCharacter::SprintDirectionLimitation(EMovementState State)
{
	if (State == EMovementState::SprintRun_State)
	{
		LookingDirection = GetActorForwardVector();
		MovingDirection = GetVelocity().GetSafeNormal();

		//Finding degrees of deviation
		DeviationCos = FVector::DotProduct(LookingDirection, MovingDirection);
		DeviationRad = FMath::Acos(DeviationCos);
		deviation = FMath::RadiansToDegrees(DeviationRad);

		if (deviation >= MaxDeviation || deviation <= -MaxDeviation)
		{
			MovementState = EMovementState::Run_State;
			CharacterUpdate();
		}
	}
}

AWeaponDefault* ATopDownShooterCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

EMovementState ATopDownShooterCharacter::GetMovementState()
{
	return MovementState;
}

TArray<UTPS_StatsEffects*> ATopDownShooterCharacter::GetCurrentEffectsOnChar()
{
	return Effects;
}

int32 ATopDownShooterCharacter::GetCurrentWeaponIndex()
{
	return CurrentIndexWeapon;
}

bool ATopDownShooterCharacter::GetIsAlive()
{
	return IsAlive;
}

void ATopDownShooterCharacter::InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo AdditionalWeaponInfo, int32 NewCurrentIndexWeapon)
{
	//On Server
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetGameInstance());
	FWeaponInfo MyWeaponInfo;

	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, MyWeaponInfo))
		{
			if (MyWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParameters.Owner = this;
				SpawnParameters.Instigator = GetInstigator();

				AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(MyWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParameters));
				if (MyWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					MyWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = MyWeapon;

					MyWeapon->WeaponSettings = MyWeaponInfo;

					MyWeapon->ReloadTime = MyWeaponInfo.ReloadTime;
					MyWeapon->UpdateStateWeapon_OnServer(MovementState);

					MyWeapon->AdditionalWeaponInfo = AdditionalWeaponInfo;

					CurrentIndexWeapon = NewCurrentIndexWeapon;

					MyWeapon->OnWeaponFire.AddDynamic(this, &ATopDownShooterCharacter::WeaponFire);
					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadEnd);

					//try reload weapon after switching if it's possible and needed
					if (CurrentWeapon->GetWeaponRound() <= 0)
						CurrentWeapon->InitReload();

					if (InventoryComponent)
						InventoryComponent->AmmoAvialableEvent_Multicast(MyWeapon->WeaponSettings.WeaponType);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATopDownShooterCharacter::InitWeapon - Weapon wasn't found in table -NULL"));
		}
	}
}

void ATopDownShooterCharacter::TrySwitchWeaponToIndexByKeyInput_OnServer_Implementation(int32 index)
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->GetWeaponSlots().IsValidIndex(index))
	{
		if (CurrentIndexWeapon != index)
		{
			int32 oldIndex = CurrentIndexWeapon;
			FAdditionalWeaponInfo oldInfo;

			oldInfo = CurrentWeapon->AdditionalWeaponInfo;
			InventoryComponent->SwitchWeaponToIndex(index, oldIndex, oldInfo);
		}
	}
}

void ATopDownShooterCharacter::DropCurrentWeapon()
{
	if (InventoryComponent)
	{
		InventoryComponent->DropWeaponByIndex_OnServer(CurrentIndexWeapon);
	}
}

void ATopDownShooterCharacter::WeaponFire(UAnimMontage* Anim)
{
	if (InventoryComponent && CurrentWeapon)
		InventoryComponent->SetAdditionalWeaponInfo(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);

	WeaponFire_BP(Anim);
}

void ATopDownShooterCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATopDownShooterCharacter::WeaponReloadEnd(bool bIsSuccess, int32 AmmoTake)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->AmmoSlotChangeValue(CurrentWeapon->WeaponSettings.WeaponType, AmmoTake); 
		InventoryComponent->SetAdditionalWeaponInfo(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}

	WeaponReloadEnd_BP(bIsSuccess);
}

void ATopDownShooterCharacter::WeaponFire_BP_Implementation(UAnimMontage* Anim)
{
	// In BluePrints
}

void ATopDownShooterCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// In BluePrints
}

void ATopDownShooterCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	//In BluePrints
}

void ATopDownShooterCharacter::CharDead_BP_Implementation()
{
	//In Blueprints
}

UDecalComponent* ATopDownShooterCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

void ATopDownShooterCharacter::SwitchNextWeapon_OnServer_Implementation()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading && InventoryComponent->GetWeaponSlots().Num() > 1)
	{
		FAdditionalWeaponInfo OldInfo;

		OldInfo = CurrentWeapon->AdditionalWeaponInfo;

		InventoryComponent->SwitchWeaponToNextOrPrevious(CurrentIndexWeapon, OldInfo, true);
	}
}

void ATopDownShooterCharacter::SwitchPreviousWeapon_OnServer_Implementation()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading &&  InventoryComponent->GetWeaponSlots().Num() > 1)
	{
		FAdditionalWeaponInfo OldInfo;

		OldInfo = CurrentWeapon->AdditionalWeaponInfo;

		InventoryComponent->SwitchWeaponToNextOrPrevious(CurrentIndexWeapon, OldInfo, false);
	}
}

void ATopDownShooterCharacter::TryAbilityEnabled()
{
	if (AbilityEffect)
	{
		UTypes::AddEffectBySurfaceType(this, NAME_None, AbilityEffect, EPhysicalSurface::SurfaceType3);
	}
}

EPhysicalSurface ATopDownShooterCharacter::GetSurfaceType()
{
	EPhysicalSurface result = EPhysicalSurface::SurfaceType_Default;

	if (HealthComponent)
		if (HealthComponent->GetShieldStrenght() <= 0)
			if (GetMesh())
			{
				UMaterialInterface* myMaterial = GetMesh()->GetMaterial(0);

				if (myMaterial)
					result = myMaterial->GetPhysicalMaterial()->SurfaceType;
			}

	return result;
}

TArray<UTPS_StatsEffects*> ATopDownShooterCharacter::GetCurrentEffects()
{
	return Effects;
}

void ATopDownShooterCharacter::RemoveEffect(UTPS_StatsEffects* EffectToRemove)
{
	Effects.Remove(EffectToRemove);

	SwitchEffect(EffectToRemove, false);
	effectToRemove = EffectToRemove;
}

void ATopDownShooterCharacter::AddEffect(UTPS_StatsEffects* EffectToAdd)
{
	Effects.Add(EffectToAdd);

	SwitchEffect(EffectToAdd, true);
	effectToAdd = EffectToAdd;
}

void ATopDownShooterCharacter::OnRep_EffectToAdd()
{
	if (effectToAdd)
		SwitchEffect(effectToAdd, true);
}

void ATopDownShooterCharacter::OnRep_EffectToRemove()
{
	if (effectToRemove)
		SwitchEffect(effectToRemove, false);
}

void ATopDownShooterCharacter::SwitchEffect(UTPS_StatsEffects* newEffect, bool bIsAdd)
{
	if (bIsAdd)
	{
		if (newEffect && newEffect->ParticleEffect)
		{
			FName nameBoneToAttach = newEffect->boneName;
			FVector location = FVector(0);

			USkeletalMeshComponent* mySkelMesh = GetMesh();
			if (mySkelMesh)
			{
				UParticleSystemComponent* newParticleSystem = UGameplayStatics::SpawnEmitterAttached(
					newEffect->ParticleEffect, mySkelMesh, nameBoneToAttach, location, 
					FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);

				particleSystemEffects.Add(newParticleSystem);
			}
		}
	}
	else
	{
		int32 i = 0;
		bool bIsFound = false;
		
		while (i < particleSystemEffects.Num() && !bIsFound)
		{
			if (particleSystemEffects[i] && newEffect->ParticleEffect && particleSystemEffects[i]->Template
				&& newEffect->ParticleEffect == particleSystemEffects[i]->Template)
			{
				bIsFound = true;
				particleSystemEffects[i]->DeactivateSystem();
				//newEffect->DestroyObject();
				particleSystemEffects[i]->DestroyComponent();
				particleSystemEffects.RemoveAt(i);
			}

			i++;
		}
	}
}

void ATopDownShooterCharacter::CharDead()
{
	float AnimTime = 0.0f;
	int8 AnimNum = FMath::RandHelper(DeadAnimations.Num());
	
	if (DeadAnimations.Num() > 0 && DeadAnimations[AnimNum] && DeadAnimations.IsValidIndex(AnimNum) && GetMesh()->GetAnimInstance())
	{
		AnimTime = DeadAnimations[AnimNum]->GetPlayLength();
		PlayAnim_Multicast(DeadAnimations[AnimNum]);
	}
	
	IsAlive = false;
	
	if (GetController())
		GetController()->UnPossess();

	GetWorldTimerManager().SetTimer(RagDollTimer, this, &ATopDownShooterCharacter::EnableRagDoll, AnimTime, false);
	GetCursorToWorld()->SetVisibility(false);

	AttackCharEvent(false);

	CharDead_BP();
}

void ATopDownShooterCharacter::EnableRagDoll()
{
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		GetMesh()->SetSimulatePhysics(true);
	}
}

float ATopDownShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsAlive)
	{
		if (!HealthComponent->GetResistToDamage())
			HealthComponent->ChangeCurrentHealth_OnServer(-DamageAmount);
	}

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		AProjectileDefault* MyProjectile = Cast<AProjectileDefault>(DamageCauser);
		if (MyProjectile)
		{
			UTypes::AddEffectBySurfaceType(this, NAME_None, 
				MyProjectile->ProjectileSetting.Effect, GetSurfaceType()); //to do NAME_None - bone for radial damage
		}
	}

	return ActualDamage;
}

void ATopDownShooterCharacter::SetActorRotationByYaw_OnServer_Implementation(float yaw)
{
	SetActorRotationByYaw_Multicast(yaw);
}

void ATopDownShooterCharacter::SetActorRotationByYaw_Multicast_Implementation(float yaw)
{
	//if (Controller && !Controller->IsLocalPlayerController())
	SetActorRotation(FQuat(FRotator(0.0f, yaw, 0.0f)));
}

void ATopDownShooterCharacter::SetMovementState_OnServer_Implementation(EMovementState newState)
{
	AimEnabled = newState == EMovementState::AimStand_State ||
		newState == EMovementState::AimWalk_State || newState == EMovementState::Aim_State;

	SetMovementState_Multicast(newState);

	ForceNetUpdate();
}

void ATopDownShooterCharacter::SetMovementState_Multicast_Implementation(EMovementState newState)
{
	MovementState = newState;
	CharacterUpdate();
}

void ATopDownShooterCharacter::TryReloadWeapon_OnServer_Implementation()
{
	if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSettings.MaxRound)
		CurrentWeapon->InitReload();
}

void ATopDownShooterCharacter::PlayAnim_Multicast_Implementation(UAnimMontage* anim)
{
	if (GetMesh() && GetMesh()->GetAnimInstance())
		GetMesh()->GetAnimInstance()->Montage_Play(anim);
}

bool ATopDownShooterCharacter::ReplicateSubobjects(UActorChannel* Channel,
	FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool wrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (int32 i = 0; i < Effects.Num(); i++)
		if (Effects[i])
			wrote |= Channel->ReplicateSubobject(Effects[i], *Bunch, *RepFlags);

	return wrote;
}

void ATopDownShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATopDownShooterCharacter, MovementState);
	DOREPLIFETIME(ATopDownShooterCharacter, AimEnabled);
	DOREPLIFETIME(ATopDownShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(ATopDownShooterCharacter, CurrentIndexWeapon);
	DOREPLIFETIME(ATopDownShooterCharacter, Effects);
	DOREPLIFETIME(ATopDownShooterCharacter, effectToAdd);
	DOREPLIFETIME(ATopDownShooterCharacter, effectToRemove);
}