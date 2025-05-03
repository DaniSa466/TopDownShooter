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
#include "TopDownShooter/Game/TopDownShooterGameInstance.h"
#include "Materials/Material.h"
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

	InitWeapon(InitWeaponName);

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
	if (CurrentWeapon)
		if (CurrentWeapon->GetWeaponRound() < CurrentWeapon->WeaponSetting.MaxRound)
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
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);
	UE_LOG(LogTemp, Warning, TEXT("Movement: AxisX = %f. AxisY = %f. Speed = %f"), AxisX, AxisY, ResSpeed);
		
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
				WalkEnabled = false;
				AimEnabled = false;
				MovementState = EMovementState::SprintRun_State;
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
		if (Stamina == 0)
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

void ATopDownShooterCharacter::InitWeapon(FName IdWeapon)
{
	UTopDownShooterGameInstance* myGI = Cast<UTopDownShooterGameInstance>(GetGameInstance());
	FWeaponInfo MyWeaponInfo;

	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeapon, MyWeaponInfo))
		{
			if (MyWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParameters.Owner = GetOwner();
				SpawnParameters.Instigator = GetInstigator();

				AWeaponDefault* MyWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(MyWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParameters));
				if (MyWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					MyWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = MyWeapon;

					MyWeapon->WeaponSetting = MyWeaponInfo;
					MyWeapon->WeaponInfo.Round = MyWeaponInfo.MaxRound;
					//Remove !!! Debug
					MyWeapon->ReloadTime = MyWeaponInfo.ReloadTime;
					MyWeapon->UpdateStateWeapon(MovementState);

					MyWeapon->OnWeaponReloadStart.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadStart);
					MyWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATopDownShooterCharacter::WeaponReloadEnd);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATopDownShooterCharacter::InitWeapon - Weapon wasn't found in table -NULL"));
		}
	}
}

void ATopDownShooterCharacter::WeaponReloadStart()
{

}

void ATopDownShooterCharacter::WeaponReloadEnd()
{

}

void ATopDownShooterCharacter::WeaponReloadStart_BP()
{
	// In BluePrints
}

void ATopDownShooterCharacter::WeaponReloadEnd_BP()
{
	//In BluePrints
}

UDecalComponent* ATopDownShooterCharacter::GetCursorToWorld()
{
	return CurrentCursor;
}
