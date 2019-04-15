// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CaptureTheFlagCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Flag.h"
#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACaptureTheFlagCharacter

ACaptureTheFlagCharacter::ACaptureTheFlagCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	GetMesh()->SetOwnerNoSee(true); // only care about the first person mesh
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACaptureTheFlagCharacter::OnBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACaptureTheFlagCharacter::OnEndOverlap);
}

void ACaptureTheFlagCharacter::BeginPlay()
{
	Super::BeginPlay();
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	CharacterMovementComponent = FindComponentByClass<UCharacterMovementComponent>();
	//ACaptureTheFlagGameState* GameState = GetWorld()->GetGameState<ACaptureTheFlagGameState>();
	//GameState->OnFlagScore.AddDynamic(this, &ACaptureTheFlagCharacter::DestroyStoredFlag);
}

void ACaptureTheFlagCharacter::Server_PickUpFlag_Implementation(AFlag* Target)
{
	Target->FlagState = EFlagState::Retained;
	UBoxComponent* BoxCollision = Target->FindComponentByClass<UBoxComponent>();
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Target->AttachToComponent
	(
		GetMesh(), 
		FAttachmentTransformRules::FAttachmentTransformRules
		(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative,
			false
		), 
		TEXT("FlagAttachPoint")
	);
	// start increment timer
	if (ACaptureTheFlagGameState* const GameState = GetWorld() != nullptr ? GetWorld()->GetGameState<ACaptureTheFlagGameState>() : nullptr)
	{
		if (ACaptureTheFlagPlayerState* CurrentPlayerState = GetPlayerState<ACaptureTheFlagPlayerState>())
		{
			GameState->OnFlagCapture.Broadcast(CurrentPlayerState->GetPlayerId(), StoredFlag);
		}
	}
}

void ACaptureTheFlagCharacter::RPC_RequestPickUpFlag_Implementation(AFlag* Target)
{
	Server_PickUpFlag(Target);
}

bool ACaptureTheFlagCharacter::RPC_RequestPickUpFlag_Validate(AFlag* Target)
{
	return true;
}

void ACaptureTheFlagCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFlag* ValidFlag = Cast<AFlag>(OtherActor))
	{
		if (ValidFlag->FlagState == EFlagState::Dropped || ValidFlag->FlagState == EFlagState::InBase)
		{
			NearbyFlag = ValidFlag;
			if (ValidFlag->BehaviorState < 0)
			{
				AttemptPickUp();
			}
		}
	}
	// add overlap vehicle here 
}

void ACaptureTheFlagCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AFlag* ValidFlag = Cast<AFlag>(OtherActor))
	{
		NearbyFlag = nullptr;
	}
}

void ACaptureTheFlagCharacter::AttemptPickUp()
{
	// pick up now
	if (IsLocallyControlled())
	{
		UStaticMeshComponent* StaticMesh = NearbyFlag->FindComponentByClass<UStaticMeshComponent>();
		StaticMesh->SetVisibility(false); // only do this locally
	}
	if (HasAuthority())
	{
		StoredFlag = NearbyFlag;
		Server_PickUpFlag(NearbyFlag);
	}
	else
	{
		StoredFlag = NearbyFlag;
		RPC_RequestPickUpFlag(NearbyFlag);
	}
}

void ACaptureTheFlagCharacter::Server_DealDamage_Implementation(const APawn* Target)
{
	// Implement the damaging here
	UE_LOG(LogFPChar, Warning, TEXT("%s Got Hit"), *this->GetName());
}

void ACaptureTheFlagCharacter::RPC_RequestHit_Implementation(const APawn* Target)
{
	Server_DealDamage(Target);
}

bool ACaptureTheFlagCharacter::RPC_RequestHit_Validate(const APawn* Target)
{
	return true;
}

void ACaptureTheFlagCharacter::Server_JumpMontage_Implementation(const bool bStop)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (bStop)
	{
		if (!CharacterMovementComponent->IsFalling())
		{
			if (JumpAnimation)
			{
				AnimInstance->Montage_Stop(0.5f, JumpAnimation);
			}
		}
	}
	else
	{
		AnimInstance->Montage_Play(JumpAnimation);
	}
}
void ACaptureTheFlagCharacter::RPC_JumpMontage_Implementation(const bool bStop)
{
	Server_JumpMontage(bStop);
}
bool ACaptureTheFlagCharacter::RPC_JumpMontage_Validate(const bool bStop)
{
	return true;
}

void ACaptureTheFlagCharacter::OnFire()
{
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
		const FVector EndLocation = SpawnLocation + (SpawnRotation.Vector() * 500.f);
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		DrawDebugLine(World, SpawnLocation, EndLocation, FColor::Blue, false, 0.2f, 0, 5.f);
		if (World->LineTraceSingleByChannel(HitResult, SpawnLocation, EndLocation, ECollisionChannel::ECC_Pawn))
		{
			ACaptureTheFlagCharacter* HitTarget = Cast<ACaptureTheFlagCharacter>(HitResult.Actor);
			if (HitTarget)
			{
				if (HasAuthority())
				{
					Server_DealDamage(HitTarget);
				}
				else
				{
					RPC_RequestHit(HitTarget);
				}
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		if (IsLocallyControlled())
		{
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}
	}
}

void ACaptureTheFlagCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACaptureTheFlagCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACaptureTheFlagCharacter::Jump()
{
	ACharacter::Jump();
	if (HasAuthority())
	{
		Server_JumpMontage(false);
	}
	else
	{
		RPC_JumpMontage(false);
	}
}

void ACaptureTheFlagCharacter::StopJump()
{
	ACharacter::StopJumping();
	if (HasAuthority())
	{
		Server_JumpMontage(true);
	}
	else
	{
		RPC_JumpMontage(true);
	}
}

void ACaptureTheFlagCharacter::Interact()
{
	if (NearbyFlag)
	{
		if (NearbyFlag->FlagState == EFlagState::Dropped || NearbyFlag->FlagState == EFlagState::InBase)
		{
			int32& BehaviorState = NearbyFlag->BehaviorState;
			if (BehaviorState == 0)
			{
				AttemptPickUp();
			}
			else if(BehaviorState > 0)
			{
				if (UWorld* const World = GetWorld())
				{
					World->GetTimerManager().SetTimer(PickUpTimer, this, &ACaptureTheFlagCharacter::AttemptPickUp, BehaviorState, false);
				}
			}
		}
	}
}

void ACaptureTheFlagCharacter::StopInteract()
{
	if (PickUpTimer.IsValid())
	{
		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PickUpTimer);
		}
	}
}
