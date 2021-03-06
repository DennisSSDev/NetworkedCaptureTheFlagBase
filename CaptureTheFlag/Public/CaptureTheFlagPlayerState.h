// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CaptureTheFlagPlayerState.generated.h"

/*
	Access all the player related data here like names, killstreaks, etc.
*/
UCLASS()
class CAPTURETHEFLAG_API ACaptureTheFlagPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	const FString GetPlayerId() const;
};
