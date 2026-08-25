// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDefault_Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

int32 DebugExplosionShow = 0;
FAutoConsoleVariableRef CVARExplodeShow(
	TEXT("TPS.DebugExplode"), DebugExplosionShow,
	TEXT("Draw Debug For Explode"), ECVF_Cheat
);

void AProjectileDefault_Grenade::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileDefault_Grenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimerExplose(DeltaTime);
}

void AProjectileDefault_Grenade::TimerExplose(float DeltaTime)
{
	// On server
	if (TimerEnabled)
	{
		if (TimerToExplose > TimeToExplose)
			Explose_OnServer();
		else
			TimerToExplose += DeltaTime;
	}
}

void AProjectileDefault_Grenade::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
	const FHitResult& Hit)
{
	Super::BulletCollisionSphereHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
	if (!TimerEnabled)
		TimerEnabled = true;
}

void AProjectileDefault_Grenade::Explose_OnServer_Implementation()
{
	// On Server

	TimerEnabled = false;

	ExploseVisual_Multicast(ProjectileSetting.ProjectileMinRadiusDamage,
		ProjectileSetting.ProjectileMaxRadiusDamage, ProjectileSetting.ExplosionFX,
		ProjectileSetting.ExplosionSound);

	TArray<AActor*> IgnoreActor;
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(),
		ProjectileSetting.ExplodeMaxDamage, ProjectileSetting.ExplodeMaxDamage * 0.2f,
		GetActorLocation(), ProjectileSetting.ProjectileMinRadiusDamage, 
		ProjectileSetting.ProjectileMaxRadiusDamage, 5,
		NULL, IgnoreActor, this, nullptr, ECC_Visibility);

	DestroyGrenade_Multicast();
}

void AProjectileDefault_Grenade::ExploseVisual_Multicast_Implementation(float minRadius, float maxRadius, 
	UParticleSystem* explosionFX, USoundBase* explosionSound)
{
	if (DebugExplosionShow)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(),
			minRadius, 12,
			FColor::Green, false, 12.f);
		DrawDebugSphere(GetWorld(), GetActorLocation(),
			maxRadius, 12,
			FColor::Red, false, 12.f);
	}

	if (explosionFX)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), explosionFX,
			GetActorLocation(), GetActorRotation(), FVector(1.f));
	if (explosionSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), explosionSound,
			GetActorLocation());
}

void AProjectileDefault_Grenade::DestroyGrenade_Multicast_Implementation()
{
	this->Destroy();
}