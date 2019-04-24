// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CaptureTheFlagCharacter.h"
#include "HoverVehicle.h"
#include "CaptureTheFlagGameMode.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	OnDeath.AddDynamic(this, &UHealthComponent::RPC_KillSelf);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<AActor>(GetOwner()); // change to generic actor
}

bool UHealthComponent::RPC_DealSpecifiedDamage_Validate(float Dmg)
{
	// The only time the health component is accessed is during shots. Those shots are validated so at this point it should be safe
	return true;
}
void UHealthComponent::RPC_DealSpecifiedDamage_Implementation(float Dmg)
{
	if ((HealthValue -= Dmg) <= 0)
	{
		OnDeath.Broadcast();
	}
}

bool UHealthComponent::RPC_DealDamage_Validate()
{
	//same as specified damage
	return true;
}
void UHealthComponent::RPC_DealDamage_Implementation()
{
	if((HealthValue -= 10) <= 0)
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Server_DealDamage_Implementation()
{
	if (HealthValue -= 10 <= 0)
	{
		OnDeath.Broadcast();
	}
}

bool UHealthComponent::RPC_KillSelf_Validate()
{
	return (HealthValue <= 0);
}
void UHealthComponent::RPC_KillSelf_Implementation()
{
	if (Owner)
	{
		if (ACaptureTheFlagCharacter* const FCharacter = Cast<ACaptureTheFlagCharacter>(Owner))
		{
			FCharacter->InstantDropFlag();
			if (ACaptureTheFlagGameMode* const GM = Cast<ACaptureTheFlagGameMode>(Owner->GetWorld()->GetAuthGameMode()))
			{
				GM->RespawnPlayer(FCharacter);
			}
		}
		else if(AHoverVehicle* HV = Cast<AHoverVehicle>(Owner))
		{
			// Get a reference to the pawn that entered the vehicle
			if (ACaptureTheFlagCharacter* SubCharacter = HV->InnerPawn)
			{
				FVector VehicleLocation = HV->GetActorLocation();
				TArray<AActor*> AttachedActors;
				SubCharacter->GetAttachedActors(AttachedActors);
				// drop the flag
				SubCharacter->InstantDropFlag();
				if (AttachedActors.Num() > 0)
				{
					AttachedActors[0]->SetActorLocation(VehicleLocation);
					AttachedActors[0]->SetActorHiddenInGame(false);
				}
				// unposses the vehicle and reposses the default pawn
				// destroy the vehicle
				// respawn player
				if (ACaptureTheFlagGameMode* const GM = Cast<ACaptureTheFlagGameMode>(Owner->GetWorld()->GetAuthGameMode()))
				{
					GM->RespawnPlayerFromVehicle(HV);
				}
			}
		}
	}
}

void UHealthComponent::Server_KillSelf_Implementation()
{
	if (Owner)
	{
		if (ACaptureTheFlagCharacter* FCharacter = Cast<ACaptureTheFlagCharacter>(Owner))
		{
			FCharacter->InstantDropFlag();
			if (ACaptureTheFlagGameMode* GM = Cast<ACaptureTheFlagGameMode>(Owner->GetWorld()->GetAuthGameMode()))
			{
				GM->RespawnPlayer(FCharacter);
			}
		}
	}
}