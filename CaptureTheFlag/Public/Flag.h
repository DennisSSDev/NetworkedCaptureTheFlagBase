// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flag.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EFlagState: uint8
{
	InBase		UMETA(DisplayName = "In Base"),
	Retained	UMETA(DisplayName = "Player has the flag"),
	Dropped		UMETA(DisplayName = "Player dropped the flag")
};

UCLASS()
class CAPTURETHEFLAG_API AFlag : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlag();

	// determine how can the player pick up the flag
	/*
		-1 Insta Pickup
		 0 Press a button to insta pick up
		>0 Wait for x time till pick up
	*/
	UPROPERTY(EditAnywhere, Replicated)
	int32 BehaviorState = -1;

	// The flag's state during gameplay
	/*
		InBase		Flag is in rest state, nobody touched it
		Retained	Flag is currently being carried by the player
		Dropped		Flag has been dropped by a player and should be returned to a base
	*/
	UPROPERTY(EditAnywhere, Replicated)
	EFlagState FlagState = EFlagState::InBase;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* FlagMesh;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* TriggerBox;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
