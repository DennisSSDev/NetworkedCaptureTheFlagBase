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

	void Jump();
	void StopJump();
	void OnFire();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Interact();
	void StopInteract();
	void DropFlag();
	void StopDropFlag();
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_PossessVehicle(AHoverVehicle* Vehicle);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_UnPossesVehicle();
protected:
	virtual void SetupInputComponent() override;
private:
	bool bHoldingJump = false;
	bool bInVehicle = false;
	class ACaptureTheFlagCharacter* DefaultPawn;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	FORCEINLINE class ACaptureTheFlagCharacter* GetDefaultPlayerPawn() const { return DefaultPawn; }
};
