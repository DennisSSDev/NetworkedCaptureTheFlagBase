// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaptureTheFlagGameState.h"
#include "FlagSpawnPoint.h"
#include "TimerManager.h"
#include "Flag.h"
#include "CaptureTheFlagGameMode.generated.h"

class ACaptureTheFlagController;

/* Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameModeEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnEvent, FVector, SpawnPosition);

// TODO: In the future, implement a team based mode
UENUM(BlueprintType)
enum class EGameType : uint8
{
	FFA		UMETA(DisplayName = "Free for All"),
	Team	UMETA(DisplayName = "Team")
};

UCLASS(minimalapi)
class ACaptureTheFlagGameMode : public AGameModeBase
{
	GENERATED_BODY()

	/* Initiates the game's restart timer (seamless travel) */
	UFUNCTION()
	void BeginGameRestart();

	UFUNCTION()
	void TickGameRestart();
	
	UFUNCTION()
	void RestartGame();
	
	UFUNCTION()
	void SendFrontEndTimer();
	
	/* Increment */
	UFUNCTION()
	void IncrementFlagCapture();

public:
	ACaptureTheFlagGameMode();

	/* How many flags a player needs to win */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	int32 WinFlagCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	float SecondsTillFlagScore = 10.f;

	/* Free for all or Team based */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	EGameType GameModeType = EGameType::FFA;

	/* Flag that will be spawned */
	UPROPERTY(EditDefaultsOnly, Category = "Flag")
	TSubclassOf<AFlag> FlagBP;

	UPROPERTY(BlueprintAssignable)
	FGameModeEvent OnGameWin;

	UPROPERTY(BlueprintAssignable)
	FSpawnEvent OnFlagSpawn;

	/* The player that currently carries the flag */
	UPROPERTY()
	FPlayerData CurrentOwningPlayer;

	/* The player that won the game */
	UPROPERTY()
	FPlayerData WonPlayer;

	/* The server side counter till player scores a flag */
	UPROPERTY()
	float FlagCaptureProgress = 0.f;

protected:
	virtual void BeginPlay() override;
	
	/* Call this upon player join in game */
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY()
	FTimerHandle GameResetTimer;
	
	UPROPERTY()
	FTimerHandle FlagCaptureTimer;

	UPROPERTY()
	float TimeTillGameRestart = -1.f;

	//All the player scores in the game. Persistent
	UPROPERTY()
	TMap<FString, int32> PlayerScores;

	//Bases
	UPROPERTY()
	TArray<AFlagSpawnPoint*> FlagSpawnPoints;

	//Flag currently being captured
	UPROPERTY()
	AFlag* StoredFlag; 

public:
	UFUNCTION()
	const float& GetTimeTillGameRestart() const;

	UFUNCTION()
	void SpawnFlag(FVector Location);
	
	UFUNCTION()
	void RespawnPlayer(APawn* Pawn);
	
	// When player exits the vehicle, move him right next to the vehicle
	UFUNCTION()
	void RespawnPlayerFromVehicle(APawn* Pawn);
	
	// Add a point to the specified player name
	UFUNCTION()
	bool AddPlayerScore(FString Name);
	
	// Start a server side flag capture timer
	UFUNCTION()
	void BeginCaptureTimer(FString PlayerName, AFlag* Flag);
	
	UFUNCTION()
	void StopCaptureTimer();

	FORCEINLINE UFUNCTION()
	int32 GetPlayerScore(FString PlayerName) const { return PlayerScores[PlayerName]; };
};