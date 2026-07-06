// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TopDownShooter/Weapon/ProjectileDefault.h"
#include "ProjectileDefault_Grenade.generated.h"

UCLASS()
class TOPDOWNSHOOTER_API AProjectileDefault_Grenade : public AProjectileDefault
{
	GENERATED_BODY()
	

protected:
	//called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//called every frame
	virtual void Tick(float DeltaTime) override;

	void TimerExplose(float DeltaTime);

	virtual void BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
		const FHitResult& Hit) override;

	virtual void ImpactProjectile() override;

	UFUNCTION(Server, Reliable)
	void Explose_OnServer();

	UFUNCTION(NetMulticast, Reliable)
	void ExploseVisual_Multicast(float minRadius, float maxRadius, 
		UParticleSystem* explosionFX, USoundBase* explosionSound);
	UFUNCTION(NetMulticast, Reliable)
	void DestroyGrenade_Multicast();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	bool TimerEnabled = false;
	float TimerToExplose = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float TimeToExplose = 5.f;
};
