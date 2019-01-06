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

	if (!bUsingGravitySnap) { CurrentHomingSpeed = 0.f; }
	if (bUsingGravitySnap) { OldVelocity = GetVelocity(); }
}
void AvrPickup::Drop()
{
	OwningMC = nullptr;
	bReadyToUse = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PickupMesh->SetSimulatePhysics(true);
}
void AvrPickup::MoveToGrabbingMC() // TODO: Figure out how to make the snapping take into account current velocity so items don't chase
{
	if (!OwningMC) { bMoving = false; return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Homing Snap
	if (!bUsingGravitySnap)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = OwningMC->GetComponentLocation();
		FVector LocationDelta = TargetLocation - CurrentLocation;

		FVector Direction = LocationDelta.GetSafeNormal();
		CurrentHomingSpeed += HomingAcceleration;
		FVector NewLocation = CurrentLocation + Direction * CurrentHomingSpeed * DeltaTime;

		SetActorLocation(NewLocation);

		FQuat StartRot = GetActorRotation().Quaternion();
		FQuat TargetRot = OwningMC->GetComponentRotation().Quaternion();
		FQuat DeltaRot = TargetRot - StartRot;
		DeltaRot *= DeltaTime / TimeToRotate;
		SetActorRotation(StartRot + DeltaRot);

		if (LocationDelta.Size() < (TargetLocation - NewLocation).Size())
		{
			bMoving = false;
			AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			bReadyToUse = true;
		}
	}
	// Gravity Snap
	if (bUsingGravitySnap)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = OwningMC->GetComponentLocation();
		FVector LocationDelta = TargetLocation - CurrentLocation;

		FVector Direction = LocationDelta.GetSafeNormal(); 
		FVector Acceleration = Direction * AttachAcceleration * DeltaTime;
		Acceleration += OldVelocity * TerminalVelocityFactor;

		SetActorLocation(CurrentLocation + Acceleration);

		FQuat StartRot = GetActorRotation().Quaternion();
		FQuat TargetRot = OwningMC->GetComponentRotation().Quaternion();
		FQuat DeltaRot = TargetRot - StartRot;
		DeltaRot *= DeltaTime / TimeToRotate;
		SetActorRotation(StartRot + DeltaRot);

		if (LocationDelta.Size() < AttachThresholdDistance)
		{
			bMoving = false;
			AttachToComponent(OwningMC, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			bReadyToUse = true;
		}

		FVector NewLocation = GetActorLocation();
		OldVelocity = NewLocation - CurrentLocation; 
	}
}

// Object Functions
void AvrPickup::TriggerPulled(float Value)
{
	if (!OwningMC || !bReadyToUse) { return; }

	BPTriggerPull(Value);
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
