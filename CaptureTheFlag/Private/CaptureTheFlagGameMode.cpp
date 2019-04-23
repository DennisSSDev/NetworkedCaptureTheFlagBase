// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CaptureTheFlagGameMode.h"
#include "CaptureTheFlagHUD.h"
#include "CaptureTheFlagCharacter.h"
#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagController.h"
#include "CaptureTheFlagPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "GameFramework/GameSession.h"
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
	OnFlagSpawn.AddDynamic(this, &ACaptureTheFlagGameMode::SpawnFlag);
}

void ACaptureTheFlagGameMode::BeginGameRestart()
{
	if (!GameResetTimer.IsValid())
	{
		TimeTillGameRestart = 5.f;
		GetWorld()->GetTimerManager().SetTimer(GameResetTimer, this, &ACaptureTheFlagGameMode::TickGameRestart, 1.f, true);
		SendFrontEndTimer();
	}
}

void ACaptureTheFlagGameMode::TickGameRestart()
{
	if (--TimeTillGameRestart < 0.f)
	{
		// Restart the game and stop the timer tick
		GetWorld()->GetTimerManager().ClearTimer(GameResetTimer);
		RestartGame();
	}
}

void ACaptureTheFlagGameMode::RestartGame()
{
	GetWorld()->SeamlessTravel("FirstPersonExampleMap");
}

void ACaptureTheFlagGameMode::SendFrontEndTimer()
{
	GetGameState<ACaptureTheFlagGameState>()->StartEndGameTimerFrontEnd();
}

void ACaptureTheFlagGameMode::SpawnFlag(FVector Location)
{
	if (FlagBP)
	{
		AFlag* SpawnedFlag = GetWorld()->SpawnActor<AFlag>
		(
			FlagBP,
			Location,
			FRotator::ZeroRotator
		);
		SpawnedFlag->BehaviorState = FMath::RandRange(-1, 3);
	}
}

const float& ACaptureTheFlagGameMode::GetTimeTillGameRestart() const
{
	return TimeTillGameRestart;
}

void ACaptureTheFlagGameMode::RespawnPlayer(APawn* Pawn)
{
	AController* const PC = Pawn->GetController();
	Pawn->Destroy();
	RestartPlayer(PC);
}

void ACaptureTheFlagGameMode::RespawnPlayerFromVehicle(APawn* Pawn)
{
	ACaptureTheFlagController* const PC = Cast<ACaptureTheFlagController>(Pawn->GetController());
	APawn* const PlayerPawn = PC->GetDefaultPlayerPawn();
	PlayerPawn->Destroy();
	Pawn->Destroy();
	RestartPlayer(PC);
}
