// Fill out your copyright notice in the Description page of Project Settings.

#include "vrPickup.h"
#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"
#include "vrPlayer.h"

AvrPickup::AvrPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("Pickup Mesh");
	PickupMesh->SetSimulatePhysics(true);
	RootComponent = PickupMesh;
}
void AvrPickup::BeginPlay()
{
	Super::BeginPlay();
	
}
void AvrPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoving) { MoveToGrabbingMC(); }
}

// Grabbing
void AvrPickup::SnapTo(UMotionControllerComponent* GrabbingController)
{
	OwningMC = GrabbingController;
	PickupMesh->SetSimulatePhysics(false);
	bMoving = true;
	CurrentTimeToAttach = TimeToAttach;
}
void AvrPickup::Drop()
{
	if (!bMoving)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	if (OwningMC)
	{
		OwningMC = nullptr;
	}

	PickupMesh->SetSimulatePhysics(true);
}
void AvrPickup::MoveToGrabbingMC()
{
	if (!OwningMC) { bMoving = false; return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = OwningMC->GetComponentLocation();
	FVector LocationDelta = TargetLocation - CurrentLocation;

	UE_LOG(LogTemp, Warning, TEXT("FIRED, time is: %f"), DeltaTime)
	SetActorLocation(CurrentLocation + LocationDelta * DeltaTime / CurrentTimeToAttach);

	CurrentTimeToAttach -= DeltaTime;

	if (LocationDelta.Size() < (LocationDelta * DeltaTime / CurrentTimeToAttach).Size())
	{
		bMoving = false;
		AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

// Interaction Functions
void AvrPickup::TriggerPulled()
{
	if (!OwningMC) { return; }

	BPTriggerPull();


}
void AvrPickup::TriggerReleased()
{
	if (!OwningMC) { return; }

	BPTriggerRelease();


}
