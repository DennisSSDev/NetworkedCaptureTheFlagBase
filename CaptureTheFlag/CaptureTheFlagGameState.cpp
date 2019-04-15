// Fill out your copyright notice in the Description page of Project Settings.

#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagGameMode.h"
#include "Engine/World.h"
#include "Flag.h"

ACaptureTheFlagGameState::ACaptureTheFlagGameState()
{
	OnFlagCapture.AddDynamic(this, &ACaptureTheFlagGameState::BeginCaptureTimer);
}

void ACaptureTheFlagGameState::Server_ScoreFlag_Implementation()
{
	PlayerScores[CurrentOwningPlayer.Name] += 1;
	ProgressBar = 0.f;
	CurrentOwningPlayer.Name = "None";
	
	//OnFlagScore.Broadcast();
	if (StoredFlag)
	{
		StoredFlag->Destroy();
	}
	// destroy current flag
	// check if the player has won
	// if yes, don't spawn flag, otherwise spawn new flag

	GetWorld()->GetTimerManager().ClearTimer(FlagCaptureTimer);

}

void ACaptureTheFlagGameState::RPC_ScoreFlag_Implementation()
{
	Server_ScoreFlag();
}

bool ACaptureTheFlagGameState::RPC_ScoreFlag_Validate()
{
	return true;
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

const float& ACaptureTheFlagGameState::GetProgressBar() const
{
	return ProgressBar;
}
