#pragma once
#include "MotionControllerComponent.h"
#include "SpecialVariables.generated.h"

/**
*	ENUM for how an object behaves when interacted with by a motion controller
*/
UENUM(BlueprintType)
enum class EGrabAction : uint8
{
	GA_ObjectToHandSnap		UMETA(DisplayName = "HandKingSnap"),
	GA_ObjectToHandAccel	UMETA(DisplayName = "HandKingAccel"),
	GA_HandToObject			UMETA(DisplayName = "ObjectKing"),
	GA_CollectObject		UMETA(DisplayName = "Collectable")
};

/**
*	ENUM for how an object is being gripped
*/
UENUM(BlueprintType)
enum class EGripStatus : uint8
{
	GS_NoGrip			UMETA(DisplayName = "NoGrip"),
	GS_SingleGrip		UMETA(DisplayName = "SingleGrip"),
	GS_DoubleGrip		UMETA(DisplayName = "DoubleGrip")
};


/**
*	Conveniently pass info about a motion controller attempting to interact with an object
*/
USTRUCT(BlueprintType)
struct FMotionControllerInfo
{
	GENERATED_BODY()

// VARIABLES
//////////////

	UPROPERTY()
	class UMotionControllerComponent* ControllerPtr;

	UPROPERTY()
	class UStaticMeshComponent* HandMeshPtr;

	UPROPERTY()
	bool bGripping = false;

// CONSTRUCTORS
/////////////////

	// Default Constructor
	FMotionControllerInfo()
	{
		ControllerPtr = nullptr;
		HandMeshPtr = nullptr;
		bGripping = false;
	}

	// Initiazlied Constructor
	FMotionControllerInfo(UMotionControllerComponent* MotionController, UStaticMeshComponent* HandMesh, bool GripState)
	{
		ControllerPtr = MotionController;
		HandMeshPtr = HandMesh;
		bGripping = GripState;
	}

};