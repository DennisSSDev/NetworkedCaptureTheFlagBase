// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class ACaptureTheFlagCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAPTURETHEFLAG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(EditAnywhere, Category="Stats")
	int32 HealthValue = 100;

	UPROPERTY()
	FHealthState OnDeath;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_DealDamage();
	UFUNCTION(NetMulticast, Reliable)
	void Server_DealDamage();

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_KillSelf();
	UFUNCTION(NetMulticast, Reliable)
	void Server_KillSelf();
private:
	ACaptureTheFlagCharacter* Owner;
};
