// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpecialVariables.h"
#include "GrabInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UGrabInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 *	For use in VR projects to allow objects to be interacted with by a motion controller
 */
class VR_PROJECT_API IGrabInterface
{
	GENERATED_BODY()

public:

// VARIABLES
//////////////

	/** This controls how the implementing actor will behave when it recieves a GrabInitiate call */
	EGrabAction GrabAction;

	AActor* OwningActor;
	bool bCanBeGrabbed = true;
	FMotionControllerInfo InteractingMC;

	float MoveRate = 20.f;
	float RotateRate = 720.f;

// FUNCTIONS
//////////////

	void InitializeGrabInterface(EGrabAction GrabBehavior, AActor* Owner);
	void ReactToGrab(const FMotionControllerInfo& GrabbingMC);
	void SnapObjectToHand();
	void AccelObjectToHand();
	void SnapHandToObject();
	void CollectObject();
	void UpdateObjectPostion(UObject* ObjectToMove, FTransform TargetTransform);
	void Drop();
};
