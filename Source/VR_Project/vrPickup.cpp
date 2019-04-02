// Copyright Aviator Games 2019

#include "vrPickup.h"
#include "Components/StaticMeshComponent.h"
#include "MotionControllerComponent.h"
#include "vrPlayer.h"
#include "Components/SceneComponent.h"
#include "vrHolster.h"
#include "vrBelt.h"
#include "SigPistol.h"

AvrPickup::AvrPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("Pickup Mesh");
	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = PickupMesh;

	SetReplicates(true);

}
void AvrPickup::BeginPlay()
{
	Super::BeginPlay();

}
void AvrPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoving && SnapTarget) { MoveTo(SnapTarget, SnapSocket); }
}

// Grabbing
void AvrPickup::SnapInitiate(USceneComponent * NewParentComponent, FName SocketName)
{
	if (!bPickupEnabled) { return; }
	bPickupEnabled = false;

	OnGrabbed.Broadcast(this);

	SnapTarget = NewParentComponent;
	SnapSocket = SocketName;

	UMotionControllerComponent* MC = Cast<UMotionControllerComponent>(NewParentComponent);
	if (MC)
	{
		OwningMC = MC;
		OwningMC->SetShowDeviceModel(false);
		OwningPlayer = Cast<AvrPlayer>(OwningMC->GetOwner());
	}
	else
	{
		OwningMC = nullptr;
		OwningPlayer = nullptr;
	}

	PickupMesh->SetSimulatePhysics(false);

	AttachToComponent(SnapTarget, FAttachmentTransformRules::KeepWorldTransform);

	bMoving = true;
	CurrentHomingSpeed = 0.f;
}

void AvrPickup::SnapOn()
{
	OnDrop.Clear();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	bMoving = false;
	bReadyToUse = true;
	AttachToComponent(SnapTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SnapSocket);
	if (OwningMC) {	OwningMC->SetShowDeviceModel(true);	}

	OnSnappedOn.Broadcast(this);
}

void AvrPickup::Drop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PickupMesh->SetSimulatePhysics(true);

	if (bReadyToUse && OwningMC) { OwningMC->SetShowDeviceModel(false); bReadyToUse = false; }
	else { bReadyToUse = false; }

	if (OwningMC) { OwningMC = nullptr; }
	if (OwningPlayer) 
	{ 
		//UE_LOG(LogTemp, Warning, TEXT("Toss velocity: %f"), (GetVelocity() - OwningPlayer->GetVelocity()).Size())
		OnDrop.Clear();

		AvrHolster* VacantHolster = Cast<AvrHolster>(OwningPlayer->GetUtilityBelt()->GetVacantHolster(this));
		if (VacantHolster && (GetVelocity() - OwningPlayer->GetVelocity()).Size() < NoHolsterSpeed && bSeeksHolster)
		{
			OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
		}
		OwningPlayer = nullptr; 
	}

	SnapTarget = nullptr;
	SnapSocket = NAME_None;
	bPickupEnabled = true;

	BPDrop();
	OnDrop.Broadcast(this);
}

void AvrPickup::MoveTo(USceneComponent * TargetComponent, FName TargetSocket)
{
	if (!TargetComponent) { bMoving = false; return; UE_LOG(LogTemp, Warning, TEXT("No SnapTarget, can't move or attach")) }

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector CurrentLocation = GetActorLocation();
	FQuat CurrentRotation = GetActorRotation().Quaternion();
	CurrentHomingSpeed += HomingAcceleration;

	FTransform TargetTransform;
	if (TargetSocket != NAME_None) { TargetTransform = TargetComponent->GetSocketTransform(TargetSocket); }
	else { TargetTransform = TargetComponent->GetComponentTransform();  }

	FVector DeltaLocation = TargetTransform.GetLocation() - CurrentLocation;
	FQuat DeltaRotation = TargetTransform.GetRotation() - CurrentRotation;

	FVector NewLocation = CurrentLocation + DeltaLocation * CurrentHomingSpeed * DeltaTime;
	FQuat NewRotation = CurrentRotation + DeltaRotation * DeltaTime / TimeToRotate;

	FVector NewDeltaLocation = NewLocation - CurrentLocation;

	if (DeltaLocation.Size() < NewDeltaLocation.Size() * 1.05f)
	{
		SnapOn();
	}
	else
	{
		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);
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

void AvrPickup::NullifySnapTarget()
{
	if (bReadyToUse && OwningMC) { OwningMC->SetShowDeviceModel(false); }
	if (OwningMC) { OwningMC = nullptr; }
	if (OwningPlayer) { OwningPlayer = nullptr; }
	SnapTarget = nullptr;
	SnapSocket = NAME_None;
	bReadyToUse = false;
	bPickupEnabled = true;
}
