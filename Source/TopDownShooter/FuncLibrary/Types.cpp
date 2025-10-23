// Fill out your copyright notice in the Description page of Project Settings.


#include "Types.h"
#include "TopDownShooter/TopDownShooter.h"

void UTypes::AddEffectBySurfaceType(AActor* TakeEffectActor, TSubclassOf<UTPS_StatsEffects> AddEffectClass, EPhysicalSurface PhysSurface)
{
	if (PhysSurface != EPhysicalSurface::SurfaceType_Default && TakeEffectActor)
	{
		UTPS_StatsEffects* myEffect = Cast<UTPS_StatsEffects>(AddEffectClass->GetDefaultObject());
		if (myEffect)
		{
			bool bCanBeAdded = false;
			int8 i = 0;

			while (i < myEffect->PossibleInteractSurface.Num() && !bCanBeAdded)
			{
				if (myEffect->PossibleInteractSurface[i] == PhysSurface)
				{
					bCanBeAdded = true;
					UTPS_StatsEffects* newEffect = NewObject<UTPS_StatsEffects>(TakeEffectActor, FName("Effect"));

					if (newEffect)
						newEffect->InitObject(TakeEffectActor);
				}
				i++;
			}
		}
	}
}
