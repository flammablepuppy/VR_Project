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

	OldVelocity = PickupMesh->GetComponentVelocity();
	
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
	CurrentAttachSpeed = AttachAcceleration;
}
void AvrPickup::Drop()
{
	OwningMC = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PickupMesh->SetSimulatePhysics(true);
}
void AvrPickup::MoveToGrabbingMC()
{
	if (!OwningMC) { bMoving = false; return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Linear acceleration version
	/*FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = OwningMC->GetComponentLocation();
	FVector LocationDelta = TargetLocation - CurrentLocation;

	SetActorLocation(CurrentLocation + LocationDelta * DeltaTime / CurrentTimeToAttach);

	CurrentTimeToAttach -= DeltaTime;

	if (LocationDelta.Size() < (LocationDelta * DeltaTime / CurrentTimeToAttach).Size())
	{
		bMoving = false;
		AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	UE_LOG(LogTemp, Warning, TEXT("FIRED, time is: %f"), DeltaTime)*/


	// Velocity based version
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = OwningMC->GetComponentLocation();
	FVector Direction = TargetLocation - CurrentLocation;
	Direction.Normalize();

	FVector Acceleration = Direction * AttachAcceleration * DeltaTime;
	FVector PickupVelocity = (OldVelocity + Acceleration) * TerminalVelocityFactor;

	SetActorLocation(CurrentLocation + PickupVelocity + Acceleration); 

	if ((TargetLocation - CurrentLocation).Size() < 1.f)
	{
		bMoving = false;
		AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	UE_LOG(LogTemp, Warning, TEXT("FIRED, speed is: "))

	FVector NewLocation = GetActorLocation();
	OldVelocity = NewLocation - CurrentLocation;
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
