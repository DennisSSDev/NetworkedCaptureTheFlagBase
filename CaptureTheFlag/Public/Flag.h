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
	// Sets default values for this actor's properties
	AFlag();
	// determine how can the player pick up the flag
	UPROPERTY(EditAnywhere)
	int32 BehaviorState = -1;
	UPROPERTY(EditAnywhere)
	EFlagState FlagState = EFlagState::InBase;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* FlagMesh;
	UPROPERTY(EditAnywhere)
	class UBoxComponent* TriggerBox;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
