// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CaptureTheFlagCharacter.h"
#include "HoverVehicle.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UHealthComponent;

UENUM()
enum class EVehicleState: uint8
{
	REST,
	POSSESED
};
UENUM()
enum class EFlagAllowance: uint8
{
	ALLOW,
	DENY
};


UCLASS()
class CAPTURETHEFLAG_API AHoverVehicle : public APawn
{
	GENERATED_BODY()

	UFUNCTION()
	void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	// Sets default values for this pawn's properties
	AHoverVehicle();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* VehicleMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TriggerBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character,  meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere)
	class UHealthComponent* HealthComponent;
	EVehicleState VehicleState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlagAllowance FlagAllowance;

	class ACaptureTheFlagCharacter* InnerPawn;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	bool bPossessed = false;
	UPROPERTY()
	float InstaKillValue = 9999.f;
public:	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_MoveForward(float Val, float Yaw);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_MoveRight(float Val, float Yaw);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestRunOverTarget(const APawn* Target);
	UFUNCTION()
	const FORCEINLINE bool& IsPossessed() const { return bPossessed; }
	UFUNCTION()
	void SetPossessed(bool bState) { bPossessed = bState; }
};