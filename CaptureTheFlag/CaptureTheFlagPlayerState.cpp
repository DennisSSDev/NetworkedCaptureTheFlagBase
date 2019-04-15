// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagPlayerState.h"

const FString ACaptureTheFlagPlayerState::GetPlayerId() const
{
	return FString::FromInt(PlayerId);
}