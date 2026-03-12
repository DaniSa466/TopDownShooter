#include "ProjectileDefault.h"
#include "Kismet/GameplayStatics.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AProjectileDefault::AProjectileDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	BulletProjectileMovement->InitialSpeed = 1.f;
	BulletProjectileMovement->MaxSpeed = 0.f;

	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AProjectileDefault::BeginPlay()
{
	Super::BeginPlay();

	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);
}

// Called every frame
void AProjectileDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileDefault::InitProjectile(FProjectileInfo InitParam)
{
	BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
	BulletProjectileMovement->MaxSpeed = InitParam.ProjectileInitSpeed;
	this->SetLifeSpan(InitParam.ProjectileLifeTime);

	if (!InitParam.Projectile)
		BulletMesh->DestroyComponent();

	if (InitParam.ProjectileTrialFX)
	{
		BulletFX->SetTemplate(ProjectileSetting.ProjectileTrialFX);
		BulletFX->SetRelativeTransform(ProjectileSetting.ProjectileTrialFXOffset);
	}
	else
		BulletFX->DestroyComponent();


	ProjectileSetting = InitParam;
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && Hit.PhysMaterial.IsValid())
	{
		EPhysicalSurface MySurfaceType = UGameplayStatics::GetSurfaceType(Hit);
		//UE_LOG(LogTemp, Warning, TEXT("AProjectileDefault::BulletCollisionSphereHit - first condition met"));

		if (ProjectileSetting.HitDecals.Contains(MySurfaceType))
		{
			UMaterialInterface* MyMaterial = ProjectileSetting.HitDecals[MySurfaceType];

			if (MyMaterial && OtherComp)
				UGameplayStatics::SpawnDecalAttached(MyMaterial, FVector(20.f), OtherComp, 
					NAME_None, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), 
					EAttachLocation::KeepWorldPosition, 10.f);
		}

		if (ProjectileSetting.HitFXs.Contains(MySurfaceType))
		{
			UParticleSystem* MyParticle = ProjectileSetting.HitFXs[MySurfaceType];

			if (MyParticle)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MyParticle, 
					FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint, 
					FVector(1.f)));
		}

		if (ProjectileSetting.HitSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.HitSound, Hit.ImpactPoint);

		UTypes::AddEffectBySurfaceType(Hit.GetActor(), Hit.BoneName, ProjectileSetting.Effect, MySurfaceType);
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileSetting.ProjectileDamage, Hit.TraceStart, Hit, GetInstigatorController(), this, NULL);
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
