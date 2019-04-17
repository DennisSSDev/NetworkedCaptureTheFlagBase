// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "CaptureTheFlagGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlagCapture, FString, PlayerName, AFlag*, Flag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFlagScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFlagDrop);
/**
 * 
 */

class AFlag;
class AFlagSpawnPoint;

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Data")
	FString Name = "None";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Data")
	int32 Score = 0;
};

UCLASS()
class CAPTURETHEFLAG_API ACaptureTheFlagGameState : public AGameStateBase
{
	GENERATED_BODY()

	UFUNCTION()
	void AddPlayerPoints();
	UFUNCTION()
	void MiscTimerTick();
	UFUNCTION(NetMulticast, Reliable)
	void Server_ScoreFlag();
public:
	ACaptureTheFlagGameState();
	UFUNCTION(BlueprintPure, Category="Capture the Flag Data")
	const FPlayerData& GetOwningPlayer() const;
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FPlayerData& GetWonPlayer() const;
	UFUNCTION(BlueprintPure, Category="Capture the Flag Data")
	const float& GetProgressBar() const;
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const float& GetTimeTillRestart() const;
	UFUNCTION()
	void BeginCaptureTimer(FString PlayerName, AFlag* Flag);
	UFUNCTION(NetMulticast, Reliable)
	void StartEndGameTimerFrontEnd();

	UPROPERTY(BlueprintAssignable)
	FFlagCapture OnFlagCapture;
	UPROPERTY(BlueprintAssignable)
	FFlagScore OnFlagScore;
	UPROPERTY(BlueprintAssignable)
	FFlagDrop OnFlagDrop;
	UPROPERTY()
	AFlag* StoredFlag;

protected:
	virtual void BeginPlay();
private:
	UPROPERTY()
	FTimerHandle FlagCaptureTimer;
	UPROPERTY()
	FTimerHandle MiscTimer; // Use for misc timer stuff
	UPROPERTY()
	FPlayerData CurrentOwningPlayer;
	UPROPERTY()
	FPlayerData WonPlayer;
	UPROPERTY()
	TMap<FString, int32> PlayerScores;
	UPROPERTY()
	float ProgressBar = 0.f;
	UPROPERTY()
	float ProgressMisc = -1.f;
	UPROPERTY()
	float SecondsTillFlagScore = 1.f;
	UPROPERTY()
    TArray<AFlagSpawnPoint*> FlagSpawnPoints;
};
