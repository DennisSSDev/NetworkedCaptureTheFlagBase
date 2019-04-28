// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CaptureTheFlagController.h"
#include "TimerManager.h"
#include "CaptureTheFlagCharacter.generated.h"

class UCharacterMovementComponent;
class UHealthComponent;

UCLASS(config=Game)
class ACaptureTheFlagCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY()
	class UCharacterMovementComponent* CharacterMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UHealthComponent* HealthComponent;
	
	/* Temporary pointer to a nearby flag */
	UPROPERTY()
	class AFlag* NearbyFlag;

	/* A flag that the actor currently has picked up */
	UPROPERTY()
	class AFlag* StoredFlag;
	
	/* A pointer to a vehicle the actor is in right now */
	UPROPERTY()
	class AHoverVehicle* StoredVehicle;

public:
	ACaptureTheFlagCharacter();

	/** Fires a projectile. */
	UFUNCTION()
	void OnFire();

	/** Handles moving forward/backward */
	UFUNCTION()
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	UFUNCTION()
	void MoveRight(float Val);

	/** Handle jump animation */
	void Jump() override;

	/** Reverse the jump state */
	UFUNCTION()
	void StopJump();

	/** Used mainly for flag interaction or vehicle hop in */
	UFUNCTION()
	void Interact();
	
	/* Stop interacting with the flag*/
	UFUNCTION()
	void StopInteract();

	/* Manually drop a flag with a timer */
	UFUNCTION()
	void DropFlag();

	/* Instant drop flag. Used internally */
	UFUNCTION()
	void InstantDropFlag();
	
	/* Cancel Flag Drop */
	UFUNCTION()
	void StopDropFlag();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

private:
	/* Play a jump animation on all machines */
	UFUNCTION(NetMulticast, Reliable)
	void Server_JumpMontage(const bool bStop);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_JumpMontage(const bool bStop);

	/* Handle hit target with weapon */
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestHit(const APawn* Target);

	/* Call when you're near a flag to initate a flag pick up process (timed) */
	UFUNCTION(NetMulticast, Reliable)
	void Server_PickUpFlag(AFlag* Target);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestPickUpFlag(AFlag* Target);

	/* Call to initiate a drop flag progress (timed) */
	UFUNCTION(NetMulticast, Reliable)
	void Server_DropFlag();
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestDropFlag();

	/* Cancels the flag capture process immediately */
	UFUNCTION(NetMulticast, Reliable)
	void Server_StopFlagCapture();

	/* Return a dropped flag to a random Base immediately */
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_ReturnFlagToBase(AFlag* Target);

	/* Draw the bullet path when shooting your weapon */
	UFUNCTION(Server, UnReliable, WithValidation)
	void RPC_DrawShot(FVector Start, FVector End);
	UFUNCTION(NetMulticast, UnReliable)
	void Server_DrawShot(FVector Start, FVector End);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/* Local function that will initiate a Server flag PICKUP request */
	UFUNCTION()
	void AttemptPickUp();

	/* Local function that will initiate a Server flag DROP request */
	UFUNCTION()
	void AttemptDropFlag();

	/* Local function that will attempt to bring the flag back to a base via server request */
	UFUNCTION()
	void AttemptRetrieveFlag();

	/* Locally invalidate the actor's ownership of the flag. Mainly used for UMG updates */
	UFUNCTION()
	void InvalidateFlagOwnership() { bHasFlag = false; }

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* JumpAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector GunOffset;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

private:
	UPROPERTY()
	FTimerHandle PickUpTimer;

	UPROPERTY()
	FTimerHandle DropTimer;

	UPROPERTY()
	float DropTime = 1.f;
	
	UPROPERTY()
	float InteractTime = 0.f;
	
	UPROPERTY()
	uint8 SpawnPointCount = 3;

	UPROPERTY()
	bool bInitPickUp = false;
	
	UPROPERTY()
	bool bInitDrop = false;
	
	UPROPERTY()
	bool bHasFlag = false;

public:
	/** Getters */

	FORCEINLINE UFUNCTION()
	class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	FORCEINLINE UFUNCTION()
	class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	FORCEINLINE UFUNCTION()
	class AFlag* GetStoredFlag() const { return StoredFlag; }

	FORCEINLINE UFUNCTION()
	class AHoverVehicle* GetStoredVehicle() const { return StoredVehicle; }
	
	/* CAN ACCESS IN BP */

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsPickUpStarted() const { return bInitPickUp; }
	
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsDropStarted() const { return bInitDrop; }

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE float& GetInteractTime() const { return InteractTime; }

	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsOwningFlag() const { return bHasFlag; }
};