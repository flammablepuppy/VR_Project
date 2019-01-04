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
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
	OldVelocity = PickupMesh->GetComponentVelocity();
}
void AvrPickup::Drop()
{
	OwningMC = nullptr;
	bReadyToUse = false;
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
	}*/

	// Velocity based version
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = OwningMC->GetComponentLocation();
	FVector Direction = TargetLocation - CurrentLocation;
	Direction.GetSafeNormal();

	FVector Acceleration = Direction * AttachAcceleration * DeltaTime;
	Acceleration += OldVelocity * TerminalVelocityFactor;

	SetActorLocation(CurrentLocation + Acceleration);

	FQuat StartRot = GetActorRotation().Quaternion();
	FQuat TargetRot = OwningMC->GetComponentRotation().Quaternion();
	FQuat DeltaRot = TargetRot - StartRot;
	DeltaRot *= DeltaTime / TimeToRotate;

	SetActorRotation(StartRot + DeltaRot);

	if ((TargetLocation - CurrentLocation).Size() < AttachThresholdDistance)
	{
		bMoving = false;
		AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bReadyToUse = true;
	}

	FVector NewLocation = GetActorLocation();
	OldVelocity = NewLocation - CurrentLocation;
}

// Object Functions
void AvrPickup::TriggerPulled()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPTriggerPull();
}
void AvrPickup::TriggerReleased()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPTriggerRelease();
}
void AvrPickup::TopPushed()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPTopPush();
}
void AvrPickup::TopReleased()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPTopRelease();
}
void AvrPickup::BottomPushed()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPBottomPush();
}
void AvrPickup::BottomReleased()
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPBottomRelease();
}

// Setters
void AvrPickup::SetPickupEnabled(bool NewState)
{
	bPickupEnabled = NewState;
}
