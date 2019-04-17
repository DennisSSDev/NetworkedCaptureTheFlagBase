// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "CaptureTheFlagGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameWin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpawn, FVector, SpawnPosition, FRotator, SpawnRotation);

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

	UFUNCTION()
	void BeginGameRestart();
	UFUNCTION()
	void TickGameRestart();
	UFUNCTION()
	void RestartGame();
public:
	ACaptureTheFlagGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	int32 WinFlagCount = 2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	float SecondsTillFlagScore = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	EGameType GameModeType = EGameType::FFA;

	FGameWin OnGameWin;
	FSpawn OnFlagSpawn;
private:
	FTimerHandle GameResetTimer;
	float TimeTillGameRestart = -1.f;
public:
	UFUNCTION()
	const float& GetTimeTillGameRestart() const;
	UFUNCTION()
	void SpawnFlag(FVector Location, FRotator Rotation) const;
};



