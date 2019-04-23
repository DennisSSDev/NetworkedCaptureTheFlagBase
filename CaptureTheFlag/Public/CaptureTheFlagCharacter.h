// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CaptureTheFlagController.h"
#include "TimerManager.h"
#include "CaptureTheFlagCharacter.generated.h"

class UInputComponent;
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
	
	UPROPERTY()
	class AFlag* NearbyFlag;
	UPROPERTY()
	class AFlag* StoredFlag;
	UPROPERTY()
	class AHoverVehicle* StoredVehicle;

public:
	ACaptureTheFlagCharacter();
	/** Fires a projectile. */
	void OnFire();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/** Handle jump animation */
	void Jump();

	/** Reverse the jump state */
	void StopJump();
	/** Used mainly for flag interaction or vehicle hop on */
	void Interact();
	void StopInteract();

	void DropFlag();
	void InstantDropFlag();
	void StopDropFlag();
protected:
	virtual void BeginPlay();
private:
	UFUNCTION(NetMulticast, Reliable)
	void Server_JumpMontage(const bool bStop);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_JumpMontage(const bool bStop);
	//UFUNCTION(NetMulticast, Reliable)
	//void Server_DealDamage(const APawn* Target);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestHit(const APawn* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Server_PickUpFlag(AFlag* Target);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestPickUpFlag(AFlag* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Server_DropFlag();
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestDropFlag();

	UFUNCTION(NetMulticast, Reliable)
	void Server_StopFlagCapture();

	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_ReturnFlagToBase(AFlag* Target);

	UFUNCTION(Server, UnReliable, WithValidation)
	void RPC_DrawShot(FVector Start, FVector End);

	UFUNCTION(NetMulticast, UnReliable)
	void Server_DrawShot(FVector Start, FVector End);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void AttemptPickUp();
	UFUNCTION()
	void AttemptDropFlag();
	UFUNCTION()
	void AttemptRetrieveFlag();
	UFUNCTION()
	void InvalidateFlagOwnership() { bHasFlag = false; }
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* JumpAnimation;
private:
	UPROPERTY()
	FTimerHandle PickUpTimer;
	UPROPERTY()
	FTimerHandle DropTimer;
	float DropTime = 1.f;
	bool bInitPickUp = false;
	bool bInitDrop = false;
	bool bHasFlag = false;
	float InteractTime = 0.f;
public:
	/** Getters */
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE class AFlag* GetStoredFlag() const { return StoredFlag; }
	FORCEINLINE class AHoverVehicle* GetStoredVehicle() const { return StoredVehicle; }
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsPickUpStarted() const { return bInitPickUp; }
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsDropStarted() const { return bInitDrop; }
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE float& GetInteractTime() const { return InteractTime; }
	UFUNCTION(BlueprintPure, Category = "Capture the Flag Data")
	const FORCEINLINE bool& IsOwningFlag() const { return bHasFlag; }
};

