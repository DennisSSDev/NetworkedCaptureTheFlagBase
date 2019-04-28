// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "CaptureTheFlagGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlagCaptureEvent, FString, PlayerName, AFlag*, Flag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFlagEvent);

class AFlag;
class AFlagSpawnPoint;

/* Main info about a player passed around */
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

	/* A function that increments the flag capture progress bar. Only client side for UMG */
	UFUNCTION()
	void IncrementLocalFlagCapture();

	UFUNCTION()
	void MiscTimerTick();

public:
	ACaptureTheFlagGameState();

	/* Client side set for UMG to see who currently carries the flag */
	UFUNCTION()
	void SetCurrentOwningPlayer(FPlayerData Player);

	/* Init the client side UMG timer to see how much progress has a player reached with flag capture */
	UFUNCTION()
	void BeginFrontEndCaptureTimer(const float BarValue, const float SecondsStandard);

	UFUNCTION()
	void InvalidateFrontEndCaptureTimer();

	/* Entry point for starting the flag capture timer (client and server) */
	UFUNCTION()
	void BeginCaptureTimer(FString PlayerName, AFlag* Flag);

	/* Invalidate client and server side flag capture timers */
	UFUNCTION()
	void StopCaptureTimer();

	/* Launch the end game timer for everyone */
	UFUNCTION(NetMulticast, Reliable)
	void StartEndGameTimerFrontEnd(const FString& WonPlayerName);

	/* Move the flag back to a base for everyone */
	UFUNCTION(NetMulticast, Reliable)
	void ReturnFlagToBase(AFlag* Target, uint8 Index);

	UPROPERTY(BlueprintAssignable)
	FFlagCaptureEvent OnFlagCapture;

	UPROPERTY(BlueprintAssignable)
	FFlagEvent OnFlagDrop;
	
	UPROPERTY(BlueprintAssignable)
	FFlagEvent OnFlagScore;

	UPROPERTY(Replicated)
	FPlayerData CurrentOwningPlayer;

protected:
	virtual void BeginPlay();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	FTimerHandle LocalFlagCaptureTimer;

	UPROPERTY()
	FTimerHandle MiscTimer; // Use for misc timer stuff

	UPROPERTY(Replicated)
	FPlayerData WonPlayer;

	UPROPERTY(Replicated)
	float ProgressBar = 0.f; // UMG progress bar

	UPROPERTY()
	float ProgressMisc = -1.f;

	UPROPERTY()
	float SecondsTillFlagScore = 1.f;

	UPROPERTY()
    TArray<AFlagSpawnPoint*> FlagSpawnPoints;

public:
	/* Getters for client side UMG visualization */
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FPlayerData& GetOwningPlayer() const;

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FPlayerData& GetWonPlayer() const;

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const float& GetProgressBar() const;

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const float& GetTimeTillRestart() const;
};
