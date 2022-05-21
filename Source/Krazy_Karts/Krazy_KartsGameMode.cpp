// Copyright Epic Games, Inc. All Rights Reserved.

#include "Krazy_KartsGameMode.h"
#include "Krazy_KartsPawn.h"
#include "Krazy_KartsHud.h"

AKrazy_KartsGameMode::AKrazy_KartsGameMode()
{
	DefaultPawnClass = AKrazy_KartsPawn::StaticClass();
	HUDClass = AKrazy_KartsHud::StaticClass();
}
