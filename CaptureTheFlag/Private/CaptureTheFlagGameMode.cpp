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
#include "HoverVehicle.h"
#include "EngineUtils.h"

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
		// do every second
		GetWorld()->GetTimerManager().SetTimer(GameResetTimer, this, &ACaptureTheFlagGameMode::TickGameRestart, 1.f, true);
		// make the clients to have visual timers too
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
	GetGameState<ACaptureTheFlagGameState>()->StartEndGameTimerFrontEnd(WonPlayer.Name);
}

void ACaptureTheFlagGameMode::IncrementFlagCapture()
{
	FlagCaptureProgress += 10.f / SecondsTillFlagScore;
	if (FlagCaptureProgress == 100.f)
	{
		AddPlayerScore(CurrentOwningPlayer.Name);
	}
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

void ACaptureTheFlagGameMode::BeginPlay()
{
	bool bAllowFlag = false;
	// Criss Cross flag allowance in the level
	for (TActorIterator<AHoverVehicle> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AHoverVehicle* HV = *ActorItr;
		if (bAllowFlag)
		{
			HV->FlagAllowance = EFlagAllowance::ALLOW;
		}
		else
		{
			HV->FlagAllowance = EFlagAllowance::DENY;
		}
		bAllowFlag = !bAllowFlag;
	}
	// Grab all the spawn points and spawn the flag
	for (TActorIterator<AFlagSpawnPoint> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		FlagSpawnPoints.Add((*ActorItr));
	}
	SpawnFlag(FlagSpawnPoints[FMath::RandRange(0, FlagSpawnPoints.Num() - 1)]->GetActorLocation());
}

void ACaptureTheFlagGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// if a player joined in as a flag is being captured -> Add a front end representation on the player's screen
	if(FlagCaptureProgress > 0.f)
	{
		if(ACaptureTheFlagGameState* const GS = GetGameState<ACaptureTheFlagGameState>())
		{
			GS->SetCurrentOwningPlayer(CurrentOwningPlayer);
			GS->InvalidateFrontEndCaptureTimer();
			GS->BeginFrontEndCaptureTimer(FlagCaptureProgress, SecondsTillFlagScore);
		}
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

bool ACaptureTheFlagGameMode::AddPlayerScore(FString Name)
{
	if (StoredFlag) // destroy current flag
	{
		StoredFlag->Destroy();
	}
	PlayerScores[Name] += 1;
	if (PlayerScores[Name] == WinFlagCount)
	{
		WonPlayer.Name = Name;
		OnGameWin.Broadcast();
		return true;
	}
	else
	{
		uint16 SpawnPointIndex = FMath::RandRange(0, FlagSpawnPoints.Num() - 1);
		OnFlagSpawn.Broadcast(FlagSpawnPoints[SpawnPointIndex]->GetActorLocation());
		StopCaptureTimer();
		return false;
	}
}

void ACaptureTheFlagGameMode::BeginCaptureTimer(FString PlayerName, AFlag* Flag)
{
	StoredFlag = Flag;
	if (FlagCaptureTimer.IsValid())
		return;
	CurrentOwningPlayer.Name = PlayerName;
	if (!PlayerScores.Contains(PlayerName))
	{
		PlayerScores.Add(PlayerName, 0);
	}
	CurrentOwningPlayer.Score = PlayerScores[PlayerName];
	GetGameState<ACaptureTheFlagGameState>()->CurrentOwningPlayer.Score = CurrentOwningPlayer.Score;
	GetWorld()->GetTimerManager().SetTimer(FlagCaptureTimer, this, &ACaptureTheFlagGameMode::IncrementFlagCapture, 0.1f, true);
}

void ACaptureTheFlagGameMode::StopCaptureTimer()
{
	if (FlagCaptureTimer.IsValid())
	{
		FlagCaptureProgress = 0.f;
		CurrentOwningPlayer.Name = "None";
		CurrentOwningPlayer.Score = -1;
		GetWorld()->GetTimerManager().ClearTimer(FlagCaptureTimer);
	}
}