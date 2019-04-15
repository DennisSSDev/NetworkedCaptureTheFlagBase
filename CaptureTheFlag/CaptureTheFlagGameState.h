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
	void BeginCaptureTimer(FString PlayerName, AFlag* Flag);
	UFUNCTION()
	void AddPlayerPoints();

	UFUNCTION(NetMulticast, Reliable)
	void Server_ScoreFlag();
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_ScoreFlag();
public:
	ACaptureTheFlagGameState();
	UFUNCTION(BlueprintPure, Category="Capture the Flag Data")
	const FPlayerData& GetOwningPlayer() const;
	UFUNCTION(BlueprintPure, Category="Capture the Flag Data")
	const float& GetProgressBar() const;
	UPROPERTY(BlueprintAssignable)
	FFlagCapture OnFlagCapture;
	FFlagScore OnFlagScore;
	FFlagDrop OnFlagDrop;
	AFlag* StoredFlag;
private:
	UPROPERTY()
	FTimerHandle FlagCaptureTimer;
	UPROPERTY()
	FPlayerData CurrentOwningPlayer;
	UPROPERTY()
	TMap<FString, int32> PlayerScores;
	UPROPERTY()
	float ProgressBar = 0.f;
	UPROPERTY()
	float SecondsTillFlagScore = 1.f;
};
