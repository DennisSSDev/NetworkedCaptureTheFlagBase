// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CaptureTheFlagController.h"
#include "TimerManager.h"
#include "CaptureTheFlagCharacter.generated.h"

class UInputComponent;
class UCharacterMovementComponent;
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
	
	UPROPERTY()
	class AFlag* NearbyFlag;
	UPROPERTY()
	class AFlag* StoredFlag;

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

protected:
	virtual void BeginPlay();
private:
	UFUNCTION(NetMulticast, Reliable)
	void Server_JumpMontage(const bool bStop);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_JumpMontage(const bool bStop);
	UFUNCTION(NetMulticast, Reliable)
	void Server_DealDamage(const APawn* Target);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestHit(const APawn* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Server_PickUpFlag(AFlag* Target);
	UFUNCTION(Server, Reliable, WithValidation)
	void RPC_RequestPickUpFlag(AFlag* Target);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void AttemptPickUp();
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
public:
	/** Getters */
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE class AFlag* GetStoredFlag() const { return StoredFlag; }
};

