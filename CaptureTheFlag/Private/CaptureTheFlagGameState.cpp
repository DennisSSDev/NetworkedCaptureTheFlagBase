// Fill out your copyright notice in the Description page of Project Settings.

#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagGameMode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Flag.h"
#include "FlagSpawnPoint.h"
#include "UnrealNetwork.h"

ACaptureTheFlagGameState::ACaptureTheFlagGameState()
{
	OnFlagCapture.AddDynamic(this, &ACaptureTheFlagGameState::BeginCaptureTimer);
	OnFlagDrop.AddDynamic(this, &ACaptureTheFlagGameState::StopCaptureTimer);
}

void ACaptureTheFlagGameState::SetCurrentOwningPlayer(FPlayerData Player)
{
	CurrentOwningPlayer = Player;
}

void ACaptureTheFlagGameState::BeginFrontEndCaptureTimer(const float BarValue, const float SecondsStandard)
{
	if (LocalFlagCaptureTimer.IsValid())
		return;
	ProgressBar = BarValue;
	SecondsTillFlagScore = SecondsStandard;
	GetWorld()->GetTimerManager().SetTimer(LocalFlagCaptureTimer, this, &ACaptureTheFlagGameState::IncrementLocalFlagCapture, 0.1f, true);
}

void ACaptureTheFlagGameState::BeginCaptureTimer(FString PlayerName, AFlag* Flag)
{
	if (LocalFlagCaptureTimer.IsValid())
		return;
	SecondsTillFlagScore = GetDefaultGameMode<ACaptureTheFlagGameMode>()->SecondsTillFlagScore;
	CurrentOwningPlayer.Name = PlayerName;
	// Luanch the server flag capture timer and then initiate the local flag timer
	if (ACaptureTheFlagGameMode* GM = (ACaptureTheFlagGameMode*)GetWorld()->GetAuthGameMode())
	{
		GM->BeginCaptureTimer(PlayerName, Flag);
		CurrentOwningPlayer.Score = GM->GetPlayerScore(PlayerName);
	}
	GetWorld()->GetTimerManager().SetTimer(LocalFlagCaptureTimer, this, &ACaptureTheFlagGameState::IncrementLocalFlagCapture, 0.1f, true); // front end only
}

void ACaptureTheFlagGameState::StopCaptureTimer()
{
	if (LocalFlagCaptureTimer.IsValid())
	{
		ProgressBar = 0.f;
		CurrentOwningPlayer.Name = "None";
		CurrentOwningPlayer.Score = -1;
		if (ACaptureTheFlagGameMode* GM = (ACaptureTheFlagGameMode*)GetWorld()->GetAuthGameMode())
		{
			GM->StopCaptureTimer();
		}
		InvalidateFrontEndCaptureTimer();
	}
}

void ACaptureTheFlagGameState::InvalidateFrontEndCaptureTimer()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LocalFlagCaptureTimer);
	}
}

void ACaptureTheFlagGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACaptureTheFlagGameState, WonPlayer);
	DOREPLIFETIME(ACaptureTheFlagGameState, CurrentOwningPlayer);
	DOREPLIFETIME(ACaptureTheFlagGameState, ProgressBar);
}

void ACaptureTheFlagGameState::ReturnFlagToBase_Implementation(AFlag* Target, uint8 Index)
{
	Target->FlagState = EFlagState::InBase;
	Target->SetActorLocation(FlagSpawnPoints[Index]->GetActorLocation());
}

void ACaptureTheFlagGameState::StartEndGameTimerFrontEnd_Implementation(const FString& WonPlayerName)
{
	WonPlayer.Name = WonPlayerName;
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

void ACaptureTheFlagGameState::IncrementLocalFlagCapture()
{
	ProgressBar += 10.f / SecondsTillFlagScore;
	if (ProgressBar == 100.f)
	{
		ProgressBar = 0.f;
		CurrentOwningPlayer.Name = "None";
		CurrentOwningPlayer.Score = -1;
		OnFlagScore.Broadcast();
		InvalidateFrontEndCaptureTimer();
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