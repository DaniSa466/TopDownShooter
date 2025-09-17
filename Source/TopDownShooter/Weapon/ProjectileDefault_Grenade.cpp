// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDefault_Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

int32 DebugExplosionShow = 1;
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
	if (TimerEnabled)
	{
		if (TimerToExplose > TimeToExplose)
			Explose();
		else
			TimerToExplose += DeltaTime;
	}
}

void AProjectileDefault_Grenade::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::BulletCollisionSphereHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
	TimerEnabled = true;
}

void AProjectileDefault_Grenade::Explose()
{
	if (DebugExplosionShow)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(),
			ProjectileSetting.ProjectileMinRadiusDamage, 12,
			FColor::Green, false, 12.f);
		DrawDebugSphere(GetWorld(), GetActorLocation(),
			ProjectileSetting.ProjectileMaxRadiusDamage, 12,
			FColor::Red, false, 12.f);
	}

	TimerEnabled = false;
	if (ProjectileSetting.ExplosionFX)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileSetting.ExplosionFX, 
			GetActorLocation(), GetActorRotation(), FVector(1.f));
	if (ProjectileSetting.ExplosionSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.ExplosionSound, 
			GetActorLocation());

	TArray<AActor*> IgnoreActor;
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(),
		ProjectileSetting.ExplodeMaxDamage, ProjectileSetting.ExplodeMaxDamage * 0.2f,
		GetActorLocation(), 1000.f, 2000.f, 5,
		NULL, IgnoreActor, this, nullptr, ECC_Visibility);
	UE_LOG(LogTemp, Warning, TEXT("AProjectileDefault_Grenade::Explose - MaxDamage = %f, MinDamage = %f"),
		ProjectileSetting.ExplodeMaxDamage, ProjectileSetting.ExplodeMaxDamage * 0.2f);

	this->Destroy();
}
