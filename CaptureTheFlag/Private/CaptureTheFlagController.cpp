// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagController.h"
#include "CaptureTheFlagCharacter.h"
#include "UnrealNetwork.h"

void ACaptureTheFlagController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ACaptureTheFlagController::Jump()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		if (!bHoldingJump)
		{
			bHoldingJump = true;
			PlayerCharacter->Jump();
		}
	}
}

void ACaptureTheFlagController::StopJump()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		bHoldingJump = false;
		PlayerCharacter->StopJump();
	}
}

void ACaptureTheFlagController::OnFire()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->OnFire();
	}
}

void ACaptureTheFlagController::MoveForward(float Value)
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->MoveForward(Value);
	}
}

void ACaptureTheFlagController::MoveRight(float Value)
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->MoveRight(Value);
	}
}

void ACaptureTheFlagController::Interact()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->Interact();
	}
}

void ACaptureTheFlagController::StopInteract()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->StopInteract();
	}
}

void ACaptureTheFlagController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// set up gameplay key bindings
	check(InputComponent);

	// Bind jump events
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACaptureTheFlagController::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACaptureTheFlagController::StopJump);

	// Bind fire event
	InputComponent->BindAction("Fire", IE_Pressed, this, &ACaptureTheFlagController::OnFire);

	// Bind Interact event
	InputComponent->BindAction("Interact", IE_Pressed, this, &ACaptureTheFlagController::Interact);
	InputComponent->BindAction("Interact", IE_Released, this, &ACaptureTheFlagController::StopInteract);

	// Bind movement events
	InputComponent->BindAxis("MoveForward", this, &ACaptureTheFlagController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACaptureTheFlagController::MoveRight);

	// "turn" handles devices that provide an absolute delta, such as a mouse.
	InputComponent->BindAxis("Turn", this, &ACaptureTheFlagController::AddYawInput);
	InputComponent->BindAxis("LookUp", this, &ACaptureTheFlagController::AddPitchInput);
}
