// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaptureTheFlagGameMode.generated.h"

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

public:
	ACaptureTheFlagGameMode();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	int32 WinFlagCount = 2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	float SecondsTillFlagScore = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Defaults")
	EGameType GameModeType = EGameType::FFA;
};



