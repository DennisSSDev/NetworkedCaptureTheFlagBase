// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CaptureTheFlagController.generated.h"
/**
 * 
 */
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

protected:
	virtual void SetupInputComponent() override;
private:
	bool bHoldingJump = false;
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
