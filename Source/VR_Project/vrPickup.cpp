// Fill out your copyright notice in the Description page of Project Settings.

#include "vrPickup.h"
#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"
#include "vrPlayer.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

AvrPickup::AvrPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("Pickup Mesh");
	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = PickupMesh;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("Physics Handle");

}
void AvrPickup::BeginPlay()
{
	Super::BeginPlay();

}
void AvrPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LastVelocityMotionController -= OwningMC->GetComponentLocation();
	if (bMoving) { MoveToGrabbingMC(); }
	LastVelocity = GetVelocity();
	LastVelocityMotionController = OwningMC->GetComponentLocation();
	UE_LOG(LogTemp, Warning, TEXT("%f"), LastVelocity.Size())
}

// Grabbing
void AvrPickup::SnapTo(UMotionControllerComponent* GrabbingController)
{
	OwningMC = GrabbingController;
	PickupMesh->SetSimulatePhysics(false);
	bMoving = true;

	CurrentHomingSpeed = LastVelocity.Size(); 
}
void AvrPickup::Drop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PickupMesh->SetSimulatePhysics(true);
	if (bReadyToUse) { SetActorLocation(OwningMC->GetComponentLocation() + OwningMC->GetForwardVector() * 10.f); }
	OwningMC = nullptr;
	bReadyToUse = false;
}
void AvrPickup::MoveToGrabbingMC() // TODO: Figure out how to make the snapping take into account current velocity so items don't chase
{
	if (!OwningMC) { bMoving = false; return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = OwningMC->GetComponentLocation() + LastVelocityMotionController;
	FVector LocationDelta = TargetLocation - CurrentLocation;

	FVector Direction = LocationDelta.GetSafeNormal();
	CurrentHomingSpeed += HomingAcceleration;
	UE_LOG(LogTemp, Warning, TEXT("CurrentSpeed: %f"), CurrentHomingSpeed)
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