// Copyright Aviator Games 2019


#include "GrabInterface.h"
#include "MotionControllerComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"

/** 
*	When a motion controller overlaps an actor implementing this interface, it will bind it to:
*	GrabInitiate delegate
*	GrabComplete delegate
*	Drop delegate
*/

UGrabInterface::UGrabInterface(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

/** 
*	Called by implementing actor to set default behavior/initialize ptrs
*/
void IGrabInterface::InitializeGrabInterface(EGrabAction GrabBehavior, AActor* Owner)
{
	GrabAction = GrabBehavior;
	OwningActor = Owner;

}

void IGrabInterface::ReactToGrab(const FMotionControllerInfo& GrabbingMC)
{
	InteractingMC = GrabbingMC;

	switch (GrabAction)
	{
		case EGrabAction::GA_ObjectToHand:
			SnapObjectToHand();
			break;

		case EGrabAction::GA_HandToObject:
			SnapHandToObject();
			break;

		case EGrabAction::GA_CollectObject:
			CollectObject();
			break;
	}
}

void IGrabInterface::SnapObjectToHand()
{
	if (!bCanBeGrabbed)
		return;

	UpdateObjectPostion(OwningActor, InteractingMC.ControllerPtr->GetComponentTransform());
}

void IGrabInterface::SnapHandToObject()
{
	if (!bCanBeGrabbed)
		return;

}

void IGrabInterface::CollectObject()
{
	if (!bCanBeGrabbed)
		return;

}

void IGrabInterface::UpdateObjectPostion(UObject* ObjectToMove, FTransform TargetTransform)
{
	FTransform CurrentTransform;
	AActor* ActorObject = Cast<AActor>(ObjectToMove);
	USceneComponent* ComponentObject = Cast<USceneComponent>(ObjectToMove);
	if (ActorObject)
	{
		CurrentTransform = ActorObject->GetActorTransform();
	}
	else if (ComponentObject)
	{
		CurrentTransform = ComponentObject->GetComponentTransform();
	}

	float DeltaSeconds = -1.f;
	if (GEngine) { DeltaSeconds = GEngine->GetWorldFromContextObject(OwningActor, EGetWorldErrorMode::ReturnNull)->GetDeltaSeconds(); }
	else { return; }
	if (DeltaSeconds < 0.f) { return; }
	
	FVector DeltaLocation = CurrentTransform.GetLocation() - TargetTransform.GetLocation();
	FQuat DeltaRotation = CurrentTransform.GetRotation() - TargetTransform.GetRotation();

	FVector NewLocation = CurrentTransform.GetLocation() + DeltaLocation * DeltaSeconds * MoveRate;
	FQuat NewRotation = CurrentTransform.GetRotation() + DeltaRotation * DeltaSeconds * RotateRate;

	if (ActorObject)
	{
		ActorObject->SetActorLocation(NewLocation);
		ActorObject->SetActorRotation(NewRotation);
		DeltaLocation = ActorObject->GetActorLocation() - TargetTransform.GetLocation();
	}
	else if (ComponentObject)
	{
		ComponentObject->SetWorldLocation(NewLocation);
		ComponentObject->SetWorldRotation(NewRotation);
		DeltaLocation = ComponentObject->GetComponentLocation() - TargetTransform.GetLocation();
	}

	if (DeltaLocation.Size() > 0.1f)
	{
		UpdateObjectPostion(ObjectToMove, TargetTransform);
	}
}

void IGrabInterface::Drop()
{

}