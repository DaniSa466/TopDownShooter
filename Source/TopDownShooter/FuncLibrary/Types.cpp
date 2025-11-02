// Fill out your copyright notice in the Description page of Project Settings.


#include "Types.h"
#include "TopDownShooter/TopDownShooter.h"
#include "TopDownShooter/Game/TPS_GameActorsInterface.h"

void UTypes::AddEffectBySurfaceType(AActor* TakeEffectActor, TSubclassOf<UTPS_StatsEffects> AddEffectClass, EPhysicalSurface PhysSurface)
{
	if (!TakeEffectActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTypes::AddEffectBySurfaceType - TakeEffectActor is NULL"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Actor: %s, Class: %s"),
		*TakeEffectActor->GetName(),
		*TakeEffectActor->GetClass()->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Implements interface: %d"),
		TakeEffectActor->GetClass()->ImplementsInterface(UTPS_GameActorsInterface::StaticClass()));

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
							EffectCanBeAdded = false;
					}
					else
						EffectCanBeAdded = false;

					if (!EffectCanBeAdded)
					{
						bHasPossibleSurface = true;
						UTPS_StatsEffects* newEffect = NewObject<UTPS_StatsEffects>(TakeEffectActor, AddEffectClass);

						if (newEffect)
							newEffect->InitObject(TakeEffectActor);
					}
				}
				i++;
			}
		}
	}
}
