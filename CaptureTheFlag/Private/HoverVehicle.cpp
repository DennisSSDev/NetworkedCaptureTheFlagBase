// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "HealthComponent.h"

// Sets default values
AHoverVehicle::AHoverVehicle()
{
	PrimaryActorTick.bCanEverTick = false;
	this->bReplicates = true;
	this->bReplicateMovement = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	VehicleMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	VehicleMesh->SetSimulatePhysics(true);

	RootComponent = VehicleMesh;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HoverCollision"));
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	CapsuleComponent->SetCapsuleHalfHeight(210.f);
	CapsuleComponent->SetCapsuleRadius(150.f);
	CapsuleComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	CapsuleComponent->SetupAttachment(VehicleMesh);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(VehicleMesh);
	CameraComponent->RelativeLocation = FVector(-300.f, 0.f, 195.f); // Position the camera
	CameraComponent->RelativeRotation = FRotator(-30.f, 0.f, 0.f);
	CameraComponent->bUsePawnControlRotation = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
	TriggerBox->SetBoxExtent(FVector(270.f, 270.f, 150.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TriggerBox->SetupAttachment(VehicleMesh);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	AddOwnedComponent(HealthComponent);

	VehicleState = EVehicleState::REST;
	FlagAllowance = EFlagAllowance::DENY;

	VehicleMesh->OnComponentHit.AddDynamic(this, &AHoverVehicle::OnCompHit);

}

void AHoverVehicle::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (UHealthComponent* HPComponent = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		float ImpactStrength = NormalImpulse.SizeSquared();
		if (ImpactStrength > 18000000000000.f)
		{
			APawn* PotentialPlayer = Cast<APawn>(OtherActor);
			RPC_RequestRunOverTarget(PotentialPlayer);
		}
	}
}

// Called when the game starts or when spawned
void AHoverVehicle::BeginPlay()
{
	Super::BeginPlay();
}

// Called to bind functionality to input
void AHoverVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AHoverVehicle::RPC_RequestRunOverTarget_Validate(const APawn* Target)
{
	//TODO: can also check the velocity
	// Check distance between the instigator and target
	if (!Target)
		return true;
	const FVector ActorLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const float DistanceToTarget = FVector::DistSquared(ActorLocation, TargetLocation);
	return (InstaKillValue == 9999.f && DistanceToTarget < 60000.f);
}

void AHoverVehicle::RPC_RequestRunOverTarget_Implementation(const APawn* Target)
{
	if (Target)
	{
		UHealthComponent* HPComponent = Target->FindComponentByClass<UHealthComponent>();
		if (HPComponent)
		{
			HPComponent->RPC_DealSpecifiedDamage(InstaKillValue); // Insta kill
		}
	}
}

bool AHoverVehicle::RPC_MoveForward_Validate(float Val, float Yaw)
{
	// check if the Val and Yaw are reasonable
	return (FMath::Abs(Val) <= 1.f && FMath::Abs(Yaw) <= 180.f);
}

void AHoverVehicle::RPC_MoveForward_Implementation(float Val, float Yaw)
{
	if (Val != 0.0f)
	{
		// add movement in that direction
		SetActorRotation(FRotator(0.f, Yaw, 0.f));
		VehicleMesh->AddImpulse(GetActorForwardVector() * Val * 30.f * VehicleMesh->GetMass());
	}
}

bool AHoverVehicle::RPC_MoveRight_Validate(float Val, float Yaw)
{
	// check if the Val and Yaw are reasonable
	return (FMath::Abs(Val) <= 1.f && FMath::Abs(Yaw) <= 180.f);
}

void AHoverVehicle::RPC_MoveRight_Implementation(float Val, float Yaw)
{
	if (Val != 0.0f)
	{
		// add movement in that direction
		SetActorRotation(FRotator(0.f, Yaw, 0.f));
		VehicleMesh->AddImpulse(GetActorRightVector() * Val * 30.f * VehicleMesh->GetMass());
	}
}

