#include "ProjectileDefault.h"
#include "Kismet/GameplayStatics.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "Perception/AISense_Damage.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AProjectileDefault::AProjectileDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	SetReplicateMovement(true);

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

	BulletCollisionSphere->SetSphereRadius(16.f);

	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);

	BulletCollisionSphere->bReturnMaterialOnMove = true;//hit event return physMaterial

	BulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation (P keybord on editor)

	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);

	BulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
	BulletFX->SetupAttachment(RootComponent);

	//BulletSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Bullet Audio"));
	//BulletSound->SetupAttachment(RootComponent);

	BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
	BulletProjectileMovement->UpdatedComponent = RootComponent;
	//BulletProjectileMovement->InitialSpeed = 1.f;
	//BulletProjectileMovement->MaxSpeed = 0.f;

	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileDefault::BeginPlay()
{
	Super::BeginPlay();
	
	if (BulletCollisionSphere)
	{
		AActor* ownerActor = GetOwner();
		APawn* instigatorPawn = GetInstigator();

		if (ownerActor)
			BulletCollisionSphere->IgnoreActorWhenMoving(ownerActor, true);

		if (instigatorPawn)
			BulletCollisionSphere->IgnoreActorWhenMoving(instigatorPawn, true);
	}
}

// Called every frame
void AProjectileDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AProjectileDefault::InitProjectile(const FProjectileInfo& InitParam)
{
	bool shootByProjectile = true;
	ProjectileSetting = InitParam;

	SetLifeSpan(InitParam.ProjectileLifeTime);

	if (!BulletProjectileMovement)
		return false;

	BulletProjectileMovement->SetVelocityInLocalSpace(FVector::ForwardVector * InitParam.ProjectileInitSpeed);

	BulletProjectileMovement->UpdateComponentVelocity();
	BulletProjectileMovement->Activate(true);

	if (InitParam.projectileStaticMesh)
	{
		InitVisualMeshProjectile_Multicast(InitParam.projectileStaticMesh, InitParam.projectileStaticMeshOffset);
	}
	else
		if (!BulletMesh)
		{
			BulletMesh->DestroyComponent();
			shootByProjectile = false;
		}

	if (InitParam.ProjectileTrialFX)
	{
		InitVisualTrailProjectile_Multicast(InitParam.ProjectileTrialFX, InitParam.ProjectileTrialFXOffset);
	}
	else
		BulletFX->DestroyComponent();


	InitVelocity_Multicast(InitParam.ProjectileInitSpeed, InitParam.ProjectileMaxSpeed);
	ProjectileSetting = InitParam;

	return shootByProjectile;
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
		return;

	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator())
		return;

	if (OtherActor && Hit.PhysMaterial.IsValid())
	{
		EPhysicalSurface MySurfaceType = UGameplayStatics::GetSurfaceType(Hit);

		if (ProjectileSetting.HitDecals.Contains(MySurfaceType))
		{
			UMaterialInterface* MyMaterial = ProjectileSetting.HitDecals[MySurfaceType];

			if (MyMaterial && OtherComp)
				SpawnHitDecal_Multicast(MyMaterial, OtherComp, Hit);
		}

		if (ProjectileSetting.HitFXs.Contains(MySurfaceType))
		{
			UParticleSystem* MyParticle = ProjectileSetting.HitFXs[MySurfaceType];

			if (MyParticle)
				SpawnHitFX_Multicast(MyParticle, Hit);
		}

		if (ProjectileSetting.HitSound)
			SpawnHitSound_Multicast(ProjectileSetting.HitSound, Hit);

		UTypes::AddEffectBySurfaceType(Hit.GetActor(), Hit.BoneName, ProjectileSetting.Effect, MySurfaceType);
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileSetting.ProjectileDamage, Hit.TraceStart, Hit, 
		GetInstigatorController(), this, NULL);
	UAISense_Damage::ReportDamageEvent(GetWorld(), Hit.GetActor(), GetInstigator(), 
		ProjectileSetting.ProjectileDamage, Hit.Location, Hit.Location);

	ImpactProjectile();
}

void AProjectileDefault::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, 
	const FHitResult& SweepResult)
{
}

void AProjectileDefault::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AProjectileDefault::ImpactProjectile()
{
	this->Destroy();
}

void AProjectileDefault::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (BulletProjectileMovement)
		BulletProjectileMovement->Velocity = NewVelocity;
}

void AProjectileDefault::InitVelocity_Multicast_Implementation(float initSpeed, float maxSpeed)
{
	if (BulletProjectileMovement)
	{
		BulletProjectileMovement->Velocity = GetActorForwardVector() * initSpeed;
		BulletProjectileMovement->InitialSpeed = initSpeed;
		BulletProjectileMovement->MaxSpeed = maxSpeed;
	}
}

void AProjectileDefault::InitVisualMeshProjectile_Multicast_Implementation(UStaticMesh* newMesh, FTransform meshRelative)
{
	BulletMesh->SetStaticMesh(newMesh);
	BulletMesh->SetRelativeTransform(meshRelative);
}

void AProjectileDefault::InitVisualTrailProjectile_Multicast_Implementation(UParticleSystem* newFX, FTransform fxRelative)
{
	BulletFX->SetTemplate(newFX);
	BulletFX->SetRelativeTransform(fxRelative);
}

void AProjectileDefault::SpawnHitDecal_Multicast_Implementation(UMaterialInterface* newDecalMaterial, 
	UPrimitiveComponent* otherComponent, FHitResult hitResult)
{
	UGameplayStatics::SpawnDecalAttached(newDecalMaterial, FVector(20.f), otherComponent, 
		NAME_None, hitResult.ImpactPoint, hitResult.ImpactNormal.Rotation(),
		EAttachLocation::KeepWorldPosition, 10.f);
}

void AProjectileDefault::SpawnHitFX_Multicast_Implementation(UParticleSystem* fxTemplate, FHitResult hitResult)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), fxTemplate,
		FTransform(hitResult.ImpactNormal.Rotation(), hitResult.ImpactPoint,
			FVector(1.f)));
}

void AProjectileDefault::SpawnHitSound_Multicast_Implementation(USoundBase* hitSound, FHitResult hitResult)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), hitSound, hitResult.ImpactPoint);
}