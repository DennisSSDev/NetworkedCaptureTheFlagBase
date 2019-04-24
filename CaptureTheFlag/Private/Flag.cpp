// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UnrealNetwork.h"

// Sets default values
AFlag::AFlag()
{
	PrimaryActorTick.bCanEverTick = false;
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 2.5f));
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = FlagMesh;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
	TriggerBox->SetBoxExtent(FVector(400.f, 400.f, 50.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TriggerBox->SetupAttachment(RootComponent);
	this->bReplicates = true;
}

// Called when the game starts or when spawned
void AFlag::BeginPlay()
{
	Super::BeginPlay();
}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFlag, BehaviorState);
}

// Called every frame
void AFlag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}