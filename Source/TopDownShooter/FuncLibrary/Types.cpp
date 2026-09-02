// Fill out your copyright notice in the Description page of Project Settings.


#include "Types.h"
#include "TopDownShooter/TopDownShooter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"

void UTypes::AddEffectBySurfaceType(AActor* TakeEffectActor, FName hitBoneName, 
	TSubclassOf<UTPS_StatsEffects> AddEffectClass, EPhysicalSurface PhysSurface)
{
	if (PhysSurface != EPhysicalSurface::SurfaceType_Default && TakeEffectActor && AddEffectClass)
	{
		UTPS_StatsEffects* myEffect = Cast<UTPS_StatsEffects>(AddEffectClass->GetDefaultObject());
		if (myEffect)
		{
			bool bHasPossibleSurface = false;
			int8 i = 0;

			while (i < myEffect->PossibleInteractSurface.Num() && !bHasPossibleSurface)
			{
				if (myEffect->PossibleInteractSurface[i] == PhysSurface)
				{
					bHasPossibleSurface = true;
					bool EffectCanBeAdded = true;
					if (!myEffect->bIsStackable)
					{
						int8 j = 0;
						TArray<UTPS_StatsEffects*> CurrentEffects;
						ITPS_GameActorsInterface* myInterface = Cast<ITPS_GameActorsInterface>(TakeEffectActor);

						if (myInterface)
							CurrentEffects = myInterface->GetCurrentEffects();

						if (CurrentEffects.Num() > 0)
						{
							while (j < CurrentEffects.Num() && EffectCanBeAdded)
							{
								if (CurrentEffects[j]->GetClass() == AddEffectClass)
									EffectCanBeAdded = false;
								j++;
							}
						}
						else
							EffectCanBeAdded = true;
					}
					else
						EffectCanBeAdded = true;

					if (EffectCanBeAdded)
					{
						bHasPossibleSurface = true;
						UTPS_StatsEffects* newEffect = NewObject<UTPS_StatsEffects>(TakeEffectActor, AddEffectClass);

						if (newEffect)
							newEffect->InitObject(TakeEffectActor, hitBoneName);
					}
				}
				i++;
			}
		}
	}
}

void UTypes::ExecuteEffectAdded(UParticleSystem* executeFX, AActor* target, 
	FVector offset, FName socket)
{
	if (target)
	{
		FName socketToAttach = socket;
		FVector location = offset;
		ACharacter* myChar = Cast<ACharacter>(target);
		if (myChar && myChar->GetMesh())
			UGameplayStatics::SpawnEmitterAttached(executeFX, myChar->GetMesh(),
				socketToAttach, location, FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget, false);
		else
			if (target->GetRootComponent())
				UGameplayStatics::SpawnEmitterAttached(executeFX, target->GetRootComponent(),
					socketToAttach, location, FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget, false);
	}
}
