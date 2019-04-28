// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthEvent);

/*
  Used to define the concept of health for a pawn
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAPTURETHEFLAG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	UPROPERTY(EditAnywhere, Category="Stats")
	int32 HealthValue = 100;

	UPROPERTY()
	FHealthEvent OnDeath;

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_DealDamage();
	UFUNCTION(NetMulticast, Reliable)
	void Server_DealDamage();

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_DealSpecifiedDamage(float Dmg);

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_KillSelf();
	UFUNCTION(NetMulticast, Reliable)
	void Server_KillSelf();

private:
	UPROPERTY()
	class AActor* Owner;
};
