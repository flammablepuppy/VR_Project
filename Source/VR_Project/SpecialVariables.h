#pragma once
#include "MotionControllerComponent.h"
#include "SpecialVariables.generated.h"

/**
*	ENUM for how an object behaves when interacted with by a motion controller
*/
UENUM(BlueprintType)
enum class EGrabAction : uint8
{
	GA_ObjectToHand		UMETA(DisplayName = "HandKing"),
	GA_HandToObject		UMETA(DisplayName = "ObjectKing"),
	GA_CollectObject	UMETA(DisplayName = "Collectable")
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

// CONSTRUCTORS
/////////////////

	// Default Constructor
	FMotionControllerInfo()
	{
		ControllerPtr = nullptr;
		HandMeshPtr = nullptr;
	}

	// Initiazlied Constructor
	FMotionControllerInfo(UMotionControllerComponent* MotionController, UStaticMeshComponent* HandMesh)
	{
		ControllerPtr = MotionController;
		HandMeshPtr = HandMesh;
	}

};