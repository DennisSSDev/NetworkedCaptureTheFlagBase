// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CaptureTheFlagCharacter.h"
#include "CaptureTheFlagGameMode.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	OnDeath.AddDynamic(this, &UHealthComponent::RPC_KillSelf);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<AActor>(GetOwner()); // change to generic actor
	// ...
}

void UHealthComponent::RPC_DealDamage_Implementation()
{
	if((HealthValue -= 10) <= 0)
	{
		OnDeath.Broadcast();
	}
}
bool UHealthComponent::RPC_DealDamage_Validate()
{
	return true;
}

void UHealthComponent::Server_DealDamage_Implementation()
{
	if (HealthValue -= 10 <= 0)
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::RPC_KillSelf_Implementation()
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

bool UHealthComponent::RPC_KillSelf_Validate()
{
	return (HealthValue <= 0);
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
