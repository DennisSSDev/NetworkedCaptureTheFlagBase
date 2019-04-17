// Fill out your copyright notice in the Description page of Project Settings.

#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagGameMode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Flag.h"
#include "FlagSpawnPoint.h"

ACaptureTheFlagGameState::ACaptureTheFlagGameState()
{
	OnFlagCapture.AddDynamic(this, &ACaptureTheFlagGameState::BeginCaptureTimer);
}

void ACaptureTheFlagGameState::Server_ScoreFlag_Implementation()
{
	PlayerScores[CurrentOwningPlayer.Name] += 1;
	
	if (StoredFlag) // destroy current flag
	{
		StoredFlag->Destroy();
	}

	if (ACaptureTheFlagGameMode* GM = (ACaptureTheFlagGameMode*)GetWorld()->GetAuthGameMode())
	{
		if (PlayerScores[CurrentOwningPlayer.Name] == GM->WinFlagCount)
		{
			WonPlayer = CurrentOwningPlayer;
			if (HasAuthority())
			{
				GM->OnGameWin.Broadcast();
			}
		}
		else
		{
			// Spawn a new flag at a random point
			if (HasAuthority())
			{
				uint16 SpawnPointIndex = FMath::RandRange(0, FlagSpawnPoints.Num() - 1);
				ACaptureTheFlagGameMode* GM = (ACaptureTheFlagGameMode*)GetWorld()->GetAuthGameMode();
				GM->OnFlagSpawn.Broadcast(FlagSpawnPoints[SpawnPointIndex]->GetActorLocation());
			}
		}
	}
	ProgressBar = 0.f;
	CurrentOwningPlayer.Name = "None";
	GetWorld()->GetTimerManager().ClearTimer(FlagCaptureTimer);
}

void ACaptureTheFlagGameState::BeginCaptureTimer(FString PlayerName, AFlag* Flag)
{
	StoredFlag = Flag;
	SecondsTillFlagScore = GetDefaultGameMode<ACaptureTheFlagGameMode>()->SecondsTillFlagScore;
	if (FlagCaptureTimer.IsValid())
		return;
	CurrentOwningPlayer.Name = PlayerName;
	if (!PlayerScores.Contains(PlayerName))
	{
		PlayerScores.Add(PlayerName, 0);
	}
	CurrentOwningPlayer.Score = PlayerScores[PlayerName];
	GetWorld()->GetTimerManager().SetTimer(FlagCaptureTimer, this, &ACaptureTheFlagGameState::AddPlayerPoints, 0.1f, true);
}

void ACaptureTheFlagGameState::StartEndGameTimerFrontEnd_Implementation()
{
	ProgressMisc = 5.f;
	GetWorld()->GetTimerManager().SetTimer(MiscTimer, this, &ACaptureTheFlagGameState::MiscTimerTick, 1.f, true);
}

void ACaptureTheFlagGameState::MiscTimerTick()
{
	if (--ProgressMisc < 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(MiscTimer);
	}
}

void ACaptureTheFlagGameState::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<AFlagSpawnPoint> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		FlagSpawnPoints.Add((*ActorItr));
	}
}

void ACaptureTheFlagGameState::AddPlayerPoints()
{
	ProgressBar += 10.f / SecondsTillFlagScore;
	if (ProgressBar == 100.f)
	{
		Server_ScoreFlag();
	}
}

const FPlayerData& ACaptureTheFlagGameState::GetOwningPlayer() const
{
	return CurrentOwningPlayer;
}

const FPlayerData& ACaptureTheFlagGameState::GetWonPlayer() const
{
	return WonPlayer;
}

const float& ACaptureTheFlagGameState::GetProgressBar() const
{
	return ProgressBar;
}

const float& ACaptureTheFlagGameState::GetTimeTillRestart() const
{
	return ProgressMisc;
}
