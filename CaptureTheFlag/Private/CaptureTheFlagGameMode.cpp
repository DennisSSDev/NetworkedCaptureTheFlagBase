// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CaptureTheFlagGameMode.h"
#include "CaptureTheFlagHUD.h"
#include "CaptureTheFlagCharacter.h"
#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagController.h"
#include "CaptureTheFlagPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Flag.h"
#include "GameFramework/GameMode.h"

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

	OnGameWin.AddDynamic(this, &ACaptureTheFlagGameMode::BeginGameRestart);
	//OnFlagSpawn.AddDynamic(this, &ACaptureTheFlagGameMode::SpawnFlag);
}

void ACaptureTheFlagGameMode::BeginGameRestart()
{
	if (!GameResetTimer.IsValid())
	{
		TimeTillGameRestart = 5.f;
		GetWorld()->GetTimerManager().SetTimer(GameResetTimer, this, &ACaptureTheFlagGameMode::TickGameRestart, 1.f, true);
	}
}

void ACaptureTheFlagGameMode::TickGameRestart()
{
	if (--TimeTillGameRestart < 0.f)
	{
		// Restart the game and stop the timer tick
		RestartGame();
		GetWorld()->GetTimerManager().ClearTimer(GameResetTimer);
	}

}

void ACaptureTheFlagGameMode::RestartGame()
{
	Cast<AGameMode>(this)->RestartGame();
}

void ACaptureTheFlagGameMode::SpawnFlag(FVector Location, FRotator Rotation) const
{
	//GetWorld()->SpawnActor<AFlag>(AFlag::StaticClass(), Location, Rotation);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	const AFlag* Flag = GetWorld()->SpawnActor<AFlag>
	(
		AFlag::StaticClass(),
		Location,
		Rotation,
		SpawnInfo
	);
}

const float& ACaptureTheFlagGameMode::GetTimeTillGameRestart() const
{
	return TimeTillGameRestart;
}
