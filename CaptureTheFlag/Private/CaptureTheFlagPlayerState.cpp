// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagPlayerState.h"

// This is a generic way to getting an identifier for a player, in the future can implement some sort of login system
const FString ACaptureTheFlagPlayerState::GetPlayerId() const
{
	return FString::FromInt(PlayerId);
}