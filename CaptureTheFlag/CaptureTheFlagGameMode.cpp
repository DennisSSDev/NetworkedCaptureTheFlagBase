// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CaptureTheFlagGameMode.h"
#include "CaptureTheFlagHUD.h"
#include "CaptureTheFlagCharacter.h"
#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagController.h"
#include "CaptureTheFlagPlayerState.h"
#include "UObject/ConstructorHelpers.h"

ACaptureTheFlagGameMode::ACaptureTheFlagGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// Register Capture the flag default classes
	HUDClass = ACaptureTheFlagHUD::StaticClass();
	GameStateClass = ACaptureTheFlagGameState::StaticClass();
	PlayerControllerClass = ACaptureTheFlagController::StaticClass();
	PlayerStateClass = ACaptureTheFlagPlayerState::StaticClass();
}
