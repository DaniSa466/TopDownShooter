// Fill out your copyright notice in the Description page of Project Settings.


#include "TPS_GameActorsInterface.h"


// Add default functionality here for any ITPS_GameActorsInterface functions that are not pure virtual.

EPhysicalSurface ITPS_GameActorsInterface::GetSurfaceType()
{
	return EPhysicalSurface::SurfaceType_Default;
}

TArray<UTPS_StatsEffects*> ITPS_GameActorsInterface::GetCurrentEffects()
{
	TArray<UTPS_StatsEffects*> effect;

	return effect;
}

void ITPS_GameActorsInterface::RemoveEffect(UTPS_StatsEffects* effectToRemove) { }

void ITPS_GameActorsInterface::AddEffect(UTPS_StatsEffects* effectToAdd) { }

