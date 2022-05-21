// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "Krazy_KartsHud.generated.h"


UCLASS(config = Game)
class AKrazy_KartsHud : public AHUD
{
	GENERATED_BODY()

public:
	AKrazy_KartsHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
