// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagController.h"
#include "CaptureTheFlagCharacter.h"
#include "HoverVehicle.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UnrealNetwork.h"

void ACaptureTheFlagController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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
	if (ACaptureTheFlagCharacter* const PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn()))
	{
		PlayerCharacter->MoveForward(Value);
	}
	else if(AHoverVehicle* const VehiclePawn = Cast<AHoverVehicle>(GetPawn()))
	{
		UCameraComponent* CameraComponent = VehiclePawn->FindComponentByClass<UCameraComponent>();

		VehiclePawn->RPC_MoveForward(Value, CameraComponent->GetComponentRotation().Yaw);
	}
}

void ACaptureTheFlagController::MoveRight(float Value)
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->MoveRight(Value);
	}
	else if (AHoverVehicle* const VehiclePawn = Cast<AHoverVehicle>(GetPawn()))
	{
		UCameraComponent* CameraComponent = VehiclePawn->FindComponentByClass<UCameraComponent>();

		VehiclePawn->RPC_MoveRight(Value, CameraComponent->GetComponentRotation().Yaw);
	}
}

void ACaptureTheFlagController::Interact()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		if (AHoverVehicle* StoredVehicle = PlayerCharacter->GetStoredVehicle())
		{
			bInVehicle = true;
			RPC_PossessVehicle(StoredVehicle);
		}
		else
		{
			PlayerCharacter->Interact();
		}
	}
	else
	{
		if (bInVehicle)
		{
			bInVehicle = false;
			RPC_UnPossesVehicle();
		}
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

void ACaptureTheFlagController::DropFlag()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->DropFlag();
	}
}

void ACaptureTheFlagController::StopDropFlag()
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->StopDropFlag();
	}
}

bool ACaptureTheFlagController::RPC_PossessVehicle_Validate(AHoverVehicle* Vehicle)
{
	// Check the distance between the instigator and the Vehicle
	return true;
}

void ACaptureTheFlagController::RPC_PossessVehicle_Implementation(AHoverVehicle* Vehicle)
{
	ACaptureTheFlagCharacter* PlayerCharacter = Cast<ACaptureTheFlagCharacter>(GetPawn());
	DefaultPawn = PlayerCharacter;
	if (Vehicle->FlagAllowance == EFlagAllowance::DENY)
	{
		PlayerCharacter->InstantDropFlag();
	}
	DefaultPawn->SetActorHiddenInGame(true);
	TArray<AActor*> AttachedActors;
	PlayerCharacter->GetAttachedActors(AttachedActors);
	if (AttachedActors.Num() > 0)
	{
		// only the flag is ever attached
		AttachedActors[0]->SetActorHiddenInGame(true);
	}
	DefaultPawn->SetActorEnableCollision(false);
	UnPossess();
	Vehicle->InnerPawn = DefaultPawn;
	Vehicle->VehicleState = EVehicleState::POSSESED;
	Possess(Vehicle);
	// Server_PossesVehicle(Vehicle);
}

bool ACaptureTheFlagController::RPC_UnPossesVehicle_Validate()
{
	return true;
}

void ACaptureTheFlagController::RPC_UnPossesVehicle_Implementation()
{
	if (DefaultPawn)
	{
		AHoverVehicle* HoverVehicle = Cast<AHoverVehicle>(GetPawn());
		FVector CurrentLocation = GetPawn()->GetActorLocation();
		DefaultPawn->SetActorLocation(CurrentLocation);
		DefaultPawn->SetActorHiddenInGame(false);
		DefaultPawn->SetActorEnableCollision(true);
		TArray<AActor*> AttachedActors;
		DefaultPawn->GetAttachedActors(AttachedActors);
		if (AttachedActors.Num() > 0)
		{
			// only the flag is ever attached
			AttachedActors[0]->SetActorHiddenInGame(false);
		}
		HoverVehicle->VehicleState = EVehicleState::REST;
		HoverVehicle->InnerPawn = nullptr;
		UnPossess();
		Possess(DefaultPawn);
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

	// Bind Drop Flag event
	InputComponent->BindAction("DropFlag", IE_Pressed, this, &ACaptureTheFlagController::DropFlag);
	InputComponent->BindAction("DropFlag", IE_Released, this, &ACaptureTheFlagController::StopDropFlag);

	// Bind movement events
	InputComponent->BindAxis("MoveForward", this, &ACaptureTheFlagController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACaptureTheFlagController::MoveRight);

	// "turn" handles devices that provide an absolute delta, such as a mouse.
	InputComponent->BindAxis("Turn", this, &ACaptureTheFlagController::AddYawInput);
	InputComponent->BindAxis("LookUp", this, &ACaptureTheFlagController::AddPitchInput);
}
