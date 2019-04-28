// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CaptureTheFlagController.generated.h"
/**
 * 
 */

class ACaptureTheFlagCharacter;

UCLASS()
class CAPTURETHEFLAG_API ACaptureTheFlagController : public APlayerController
{
	GENERATED_BODY()

	UFUNCTION()
	void Jump();

	UFUNCTION()
	void StopJump();

	/* Causes the player to shoot his weapon */
	UFUNCTION()
	void OnFire();

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	/* Causes the player to interact with interactable objects (flag + vehicle) */
	UFUNCTION()
	void Interact();

	UFUNCTION()
	void StopInteract();

	UFUNCTION()
	void DropFlag();

	UFUNCTION()
	void StopDropFlag();

	/* Attempt a vehicle possession */
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_PossessVehicle(AHoverVehicle* Vehicle);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_UnPossesVehicle();

protected:
	/* Bind all the input here */
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	class ACaptureTheFlagCharacter* DefaultPawn;

	UPROPERTY()
	uint8 bHoldingJump : 1;

	UPROPERTY()
	uint8 bInVehicle   : 1;

public:
	FORCEINLINE UFUNCTION()
	class ACaptureTheFlagCharacter* GetDefaultPlayerPawn() const { return DefaultPawn; }
};
