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
#include "HealthComponent.h"
#include "CaptureTheFlagGameState.h"
#include "CaptureTheFlagGameMode.h"
#include "CaptureTheFlagPlayerState.h"
#include "HoverVehicle.h"

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

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	AddOwnedComponent(HealthComponent);
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
	bInitPickUp = false;
	bInitDrop = false;
	InteractTime = 0.f;
	if (ACaptureTheFlagGameState* const GameState = GetWorld() != nullptr ? GetWorld()->GetGameState<ACaptureTheFlagGameState>() : nullptr)
	{
		GameState->OnFlagScore.AddDynamic(this, &ACaptureTheFlagCharacter::InvalidateFlagOwnership);
	}
}

void ACaptureTheFlagCharacter::Server_PickUpFlag_Implementation(AFlag* Target)
{
	if (!Target)
		return;
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
			GameState->OnFlagCapture.Broadcast(CurrentPlayerState->GetPlayerId(), Target);
		}
	}
}

void ACaptureTheFlagCharacter::RPC_RequestPickUpFlag_Implementation(AFlag* Target)
{
	Server_PickUpFlag(Target);
}
bool ACaptureTheFlagCharacter::RPC_RequestPickUpFlag_Validate(AFlag* Target)
{
	// distance to flag 
	// check whether Target behavior state is -1, 0, 1, 2 or 3
	if (!Target)
	{
		return true;
	}
	else
	{
		const int32& FlagBehaviorState = Target->BehaviorState;
		const FVector ActorLocation = GetActorLocation();
		const FVector FlagLocation = Target->GetActorLocation();
		const float DistanceToFlag = FVector::DistSquared(ActorLocation, FlagLocation);
		if (FlagBehaviorState > 3 || FlagBehaviorState < -1)
		{
			return false;
		}
		else if(DistanceToFlag > 27000.f)
		{
			return false;
		}
		return true;
	}
}

void ACaptureTheFlagCharacter::Server_DropFlag_Implementation()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	bHasFlag = false;
	if (AttachedActors.Num() > 0)
	{
		AFlag* AttachedFlag = Cast<AFlag>(AttachedActors[0]);
		if (AttachedFlag)
		{
			FDetachmentTransformRules Rules
			(
				EDetachmentRule::KeepWorld, 
				EDetachmentRule::KeepRelative, 
				EDetachmentRule::KeepRelative, 
				true
			);
			AttachedFlag->DetachFromActor(Rules);
			AttachedFlag->SetActorRotation(FRotator::ZeroRotator);
			AttachedFlag->SetActorScale3D(FVector(0.2f, 0.2f, 2.5f));
			AttachedFlag->FlagState = EFlagState::Dropped;
			UStaticMeshComponent* StaticMesh = AttachedFlag->FindComponentByClass<UStaticMeshComponent>();
			StaticMesh->SetVisibility(true);
			AttachedFlag->BehaviorState = FMath::RandRange(0, 3);
			UBoxComponent* BoxCollision = AttachedFlag->FindComponentByClass<UBoxComponent>();
			BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			if (ACaptureTheFlagGameState* const GameState = GetWorld() != nullptr ? 
				GetWorld()->GetGameState<ACaptureTheFlagGameState>() 
				: 
				nullptr)
			{
				GameState->OnFlagDrop.Broadcast();
			}
		}
	}
}

bool ACaptureTheFlagCharacter::RPC_RequestDropFlag_Validate()
{
	// Check if I have a flag attached
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	if (AttachedActors.Num() > 0)
		return true;
	return false;
}
void ACaptureTheFlagCharacter::RPC_RequestDropFlag_Implementation()
{
	Server_DropFlag();
}

void ACaptureTheFlagCharacter::Server_StopFlagCapture_Implementation()
{
	if (ACaptureTheFlagGameState* const GameState = GetWorld() != nullptr ? 
		GetWorld()->GetGameState<ACaptureTheFlagGameState>() 
		: 
		nullptr)
	{
		GameState->OnFlagDrop.Broadcast();
	}
}

bool ACaptureTheFlagCharacter::RPC_ReturnFlagToBase_Validate(AFlag* Target)
{
	// check if the Target is in drop state
	// and that you're near the flag
	const FVector ActorLocation = GetActorLocation();
	const FVector FlagLocation = Target->GetActorLocation();
	const float DistanceToFlag = FVector::DistSquared(ActorLocation, FlagLocation);
	return (Target->FlagState != EFlagState::Dropped || DistanceToFlag > 27000.f) ? false : true;
}
void ACaptureTheFlagCharacter::RPC_ReturnFlagToBase_Implementation(AFlag* Target)
{
	if(ACaptureTheFlagGameState* GameState = GetWorld()->GetGameState<ACaptureTheFlagGameState>())
	{
		uint8 SpawnPointIndex = FMath::RandRange(0, 2); // TODO: this is hard coded for now
		Target->BehaviorState = FMath::RandRange(-1, 3);
		GameState->ReturnFlagToBase(Target, SpawnPointIndex);
	}
}

bool ACaptureTheFlagCharacter::RPC_DrawShot_Validate(FVector Start, FVector End)
{
	// This is just a draw call. No essential point to validate
	return true;
}
void ACaptureTheFlagCharacter::RPC_DrawShot_Implementation(FVector Start, FVector End)
{
	Server_DrawShot(Start, End);
}

void ACaptureTheFlagCharacter::Server_DrawShot_Implementation(FVector Start, FVector End)
{
	DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.2f, 0, 5.f);
}

void ACaptureTheFlagCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFlag* ValidFlag = Cast<AFlag>(OtherActor))
	{
		if (ValidFlag->FlagState == EFlagState::Dropped || ValidFlag->FlagState == EFlagState::InBase) // TODO: FIx this
		{
			NearbyFlag = ValidFlag;
			if (ValidFlag->FlagState == EFlagState::InBase)
			{
				if (ValidFlag->BehaviorState < 0)
				{
					AttemptPickUp();
				}
			}
		}
	}
	else if (AHoverVehicle* ValidHoverVehicle = Cast<AHoverVehicle>(OtherActor))
	{
		if (ValidHoverVehicle->VehicleState == EVehicleState::REST)
		{
			// set reference
			StoredVehicle = ValidHoverVehicle;
		}
	}
	// add overlap vehicle here 
}

void ACaptureTheFlagCharacter::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AFlag* ValidFlag = Cast<AFlag>(OtherActor))
	{
		NearbyFlag = nullptr;
		InteractTime = 0.f;
		bInitPickUp = false;
		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PickUpTimer);
		}
	}
	else if(AHoverVehicle* ValidHoverVehicle = Cast<AHoverVehicle>(OtherActor))
	{
		StoredVehicle = nullptr;
	}
}

void ACaptureTheFlagCharacter::AttemptPickUp()
{
	if (bHasFlag)
		return;
	if (IsLocallyControlled())
	{
		InteractTime = 0.f;
		bInitPickUp = false;
		bHasFlag = true;
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

void ACaptureTheFlagCharacter::AttemptDropFlag()
{
	StoredFlag = nullptr;
	if(IsLocallyControlled())
	{
		InteractTime = 0.f;
		bInitDrop = false;
		bHasFlag = false;
	}
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	if (AttachedActors.Num() > 0)
	{
		if (HasAuthority())
		{
			Server_DropFlag();
		}
		else
		{
			RPC_RequestDropFlag();
		}
	}
}

void ACaptureTheFlagCharacter::AttemptRetrieveFlag()
{
	InteractTime = 0.f;
	bInitPickUp = false;
	RPC_ReturnFlagToBase(NearbyFlag);
}

void ACaptureTheFlagCharacter::RPC_RequestHit_Implementation(const APawn* Target)
{
	UHealthComponent* HPComponent = Target->FindComponentByClass<UHealthComponent>();
	if (HPComponent)
	{
		HPComponent->RPC_DealDamage();
	}
}

bool ACaptureTheFlagCharacter::RPC_RequestHit_Validate(const APawn* Target)
{
	// check that the hit could've happened (actor rotation to the pawn)
	// check the distance between the target and the instigator
	const FVector InstigatorForward = GetActorForwardVector();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector InstigatorLocation = GetActorLocation();

	FVector TargetDirection = TargetLocation - InstigatorLocation;
	TargetDirection.Normalize();
	
	float Dot = FVector::DotProduct(InstigatorForward, TargetDirection);
	Dot = FMath::Acos(Dot);
	const float Distance = FVector::DistSquared(TargetLocation, InstigatorLocation);
	return (Dot < 0.18f && Distance < 500000.f);
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
		const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? 
			FP_MuzzleLocation->GetComponentLocation() 
			: 
			GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
		const FVector EndLocation = SpawnLocation + (SpawnRotation.Vector() * 500.f);
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		if (HasAuthority())
		{
			Server_DrawShot(SpawnLocation, EndLocation);
		}
		else
		{
			RPC_DrawShot(SpawnLocation, EndLocation);
		}
		if (World->LineTraceSingleByChannel(HitResult, SpawnLocation, EndLocation, ECollisionChannel::ECC_Pawn))
		{
			APawn* HitTarget = Cast<APawn>(HitResult.Actor);
			if (HitTarget)
			{
				RPC_RequestHit(HitTarget);
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
		int32& BehaviorState = NearbyFlag->BehaviorState;
		if (NearbyFlag->FlagState == EFlagState::InBase)
		{
			if (bHasFlag)
				return;
			if (BehaviorState == 0)
			{
				AttemptPickUp();
			}
			else if(BehaviorState > 0)
			{
				if (UWorld* const World = GetWorld())
				{
					InteractTime = BehaviorState;
					bInitPickUp = true;
					World->GetTimerManager().SetTimer
					(
						PickUpTimer, 
						this, 
						&ACaptureTheFlagCharacter::AttemptPickUp, 
						BehaviorState, 
						false
					);
				}
			}
		}
		else if (NearbyFlag->FlagState == EFlagState::Dropped)
		{
			if (BehaviorState == 0)
			{
				RPC_ReturnFlagToBase(NearbyFlag);
			}
			else
			{
				if (UWorld* const World = GetWorld())
				{
					InteractTime = BehaviorState;
					bInitPickUp = true;
					World->GetTimerManager().SetTimer
					(
						PickUpTimer, 
						this, 
						&ACaptureTheFlagCharacter::AttemptRetrieveFlag, 
						BehaviorState, 
						false
					);
				}
			}
		}
	}
}

void ACaptureTheFlagCharacter::StopInteract()
{
	InteractTime = 0.f;
	bInitPickUp = false;
	if (PickUpTimer.IsValid())
	{
		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PickUpTimer);
		}
	}
}

void ACaptureTheFlagCharacter::DropFlag()
{
	if(StoredFlag)
	{
		if (UWorld* const World = GetWorld())
		{
			InteractTime = DropTime;
			bInitDrop = true;
			World->GetTimerManager().SetTimer
			(
				DropTimer, 
				this, 
				&ACaptureTheFlagCharacter::AttemptDropFlag, 
				DropTime, 
				false
			);
		}
	}
}

void ACaptureTheFlagCharacter::InstantDropFlag()
{
	AttemptDropFlag();
}

void ACaptureTheFlagCharacter::StopDropFlag()
{
	InteractTime = 0.f;
	bInitDrop = false;
	if (DropTimer.IsValid())
	{
		if (UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DropTimer);
		}
	}
}