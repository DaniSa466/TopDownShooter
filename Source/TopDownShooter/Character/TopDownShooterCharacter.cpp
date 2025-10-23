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
#include "TopDownShooter/Game/TopDownShooterGameInstance.h"
#include "TopDownShooter/Weapon/ProjectileDefault.h"
#include "Materials/Material.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"

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
}

void ATopDownShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ChangeMovementState();
	StaminaSystem(MovementState);
	SprintDirectionLimitation(MovementState);
	MovementTick(DeltaSeconds);

	if(CurrentCursor)
	{
		APlayerController* myPC = Cast<APlayerController>(GetController());
		if (myPC)
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

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
}

void ATopDownShooterCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATopDownShooterCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATopDownShooterCharacter::InputAxisY);

	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATopDownShooterCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATopDownShooterCharacter::InputAttackReleased);
	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &ATopDownShooterCharacter::TryReloadWeapon);

	NewInputComponent->BindAction(TEXT("SwitchNextWeapon"), EInputEvent::IE_Pressed, this, &ATopDownShooterCharacter::SwitchNextWeapon);
	NewInputComponent->BindAction(TEXT("SwitchPreviousWeapon"), EInputEvent::IE_Pressed, this, &ATopDownShooterCharacter::SwitchPreviousWeapon);
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
	AttackCharEvent(true);
}

void ATopDownShooterCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void ATopDownShooterCharacter::TryReloadWeapon()
{
	if (CurrentWeapon && !CurrentWeapon->WeaponReloading)
		if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSettings.MaxRound)
			CurrentWeapon->InitReload();
}

void ATopDownShooterCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (myWeapon)
		myWeapon->SetWeaponStateFire(bIsFiring);

	else
		UE_LOG(LogTemp, Warning, TEXT("ATopDownShooterCharacter::AttackCharEvent - CurrentWeapon - NULL"));
}

void ATopDownShooterCharacter::MovementTick(float DeltaTime)
{
	if (IsAlive)
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

				CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
			}
		}

		if (CurrentWeapon)
			if (FMath::IsNearlyZero(GetVelocity().Size(), 0.5f))
				CurrentWeapon->ShouldReduceDispersion = true;
			else
				CurrentWeapon->ShouldReduceDispersion = false;
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

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void ATopDownShooterCharacter::ChangeMovementState()
{
	if (AxisX != 0 || AxisY != 0)
	{
		if (!WalkEnabled && !SprintRunEnabled && !AimEnabled)
			MovementState = EMovementState::Run_State;

		else
		{
			if (SprintRunEnabled)
			{
				if (Stamina > 0)
				{
					WalkEnabled = false;
					AimEnabled = false;
					MovementState = EMovementState::SprintRun_State;
				}
				else
					SprintRunEnabled = false;
			}

			else if (WalkEnabled && !SprintRunEnabled && AimEnabled)
				MovementState = EMovementState::AimWalk_State;

			else
			{
				if (WalkEnabled && !SprintRunEnabled && !AimEnabled)
					MovementState = EMovementState::Walk_State;

				else
					MovementState = EMovementState::Aim_State;
			}
		}
	}

	else
	{
		if (AimEnabled)
			MovementState = EMovementState::AimStand_State;
		else
			MovementState = EMovementState::Stand_State;
	}

	CharacterUpdate();

	//Weapon state update
	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
		myWeapon->UpdateStateWeapon(MovementState);
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

void ATopDownShooterCharacter::InitWeapon(FName IdWeaponName, FAdditionalWeaponInfo AdditionalWeaponInfo, int32 NewCurrentIndexWeapon)
{
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
					MyWeapon->UpdateStateWeapon(MovementState);

					MyWeapon->AdditionalWeaponInfo = AdditionalWeaponInfo;

					CurrentIndexWeapon = NewCurrentIndexWeapon;

					MyWeapon->OnWeaponFire.AddDynamic(this, &ATopDownShooterCharacter::WeaponFire);
					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadEnd);

					//try reload weapon after switching if it's possible and needed
					if (CurrentWeapon->GetWeaponRound() <= 0)
						CurrentWeapon->InitReload();

					if (InventoryComponent)
						InventoryComponent->OnAmmoAvialable.Broadcast(MyWeapon->WeaponSettings.WeaponType);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATopDownShooterCharacter::InitWeapon - Weapon wasn't found in table -NULL"));
		}
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

UDecalComponent* ATopDownShooterCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}

void ATopDownShooterCharacter::SwitchNextWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		FAdditionalWeaponInfo OldInfo;

		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon, OldInfo, true);
	}
}

void ATopDownShooterCharacter::SwitchPreviousWeapon()
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		FAdditionalWeaponInfo OldInfo;

		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon, OldInfo, false);
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

void ATopDownShooterCharacter::CharDead()
{
	float AnimTime = 0.0f;
	int8 AnimNum = FMath::RandHelper(DeadAnimation.Num());
	
	if (DeadAnimation[AnimNum] && DeadAnimation.IsValidIndex(AnimNum) && GetMesh()->GetAnimInstance())
	{
		AnimTime = DeadAnimation[AnimNum]->GetPlayLength();
		GetMesh()->GetAnimInstance()->Montage_Play(DeadAnimation[AnimNum]);
	}
	
	IsAlive = false;
	UnPossessed();
	
	GetWorldTimerManager().SetTimer(RagDollTimer, this, &ATopDownShooterCharacter::EnableRagDoll, AnimTime, false);
	GetCursorToWorld()->SetVisibility(false);
}

void ATopDownShooterCharacter::EnableRagDoll()
{
	UE_LOG(LogTemp, Warning, TEXT("ATPS_Character::EnableRagDoll executes."));

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
		HealthComponent->ChangeCurrentHealth(-DamageAmount);

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		AProjectileDefault* MyProjectile = Cast<AProjectileDefault>(DamageCauser);
		if (MyProjectile)
		{
			UTypes::AddEffectBySurfaceType(this, MyProjectile->ProjectileSetting.Effect, GetSurfaceType());
		}
	}

	return ActualDamage;
}
