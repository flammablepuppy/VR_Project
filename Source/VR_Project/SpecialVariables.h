#pragma once
#include "MotionControllerComponent.h"
#include "SpecialVariables.generated.h"

/**
*	ENUM for how an object behaves when interacted with by a motion controller
*/
UENUM(BlueprintType)
enum class EGrabAction : uint8
{
	GA_ObjectToHandSnap		UMETA(DisplayName = "HandSnap"),
	GA_ObjectToHandAccel	UMETA(DisplayName = "HandAccel"),
	GA_HandToObject			UMETA(DisplayName = "ObjectAccel"),
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

USTRUCT(BlueprintType)
struct FCombatEffect
{
	GENERATED_BODY()

	UPROPERTY()
	FString EffectTag;

	UPROPERTY()
	float EffectDuration;

	UPROPERTY()
	int32 EffectTicks;

	UPROPERTY()
	float EffectPower;

	UPROPERTY()
	FDamageEvent DamEve;

	UPROPERTY()
	AController* EffectInstigator;

	UPROPERTY()
	AActor* EffectCauser;


	FCombatEffect()
	{
		EffectTag = "";
		EffectDuration = -1.f;
		EffectTicks = 3;
		EffectPower = 25.f;
		DamEve = FDamageEvent();
		EffectInstigator = nullptr;
		EffectCauser = nullptr;
	}

	FCombatEffect(FString Tag, float Power = 25.f, float Duration = -1.f, int32 TicksOfEffect = 3, AController* Instigator = nullptr, AActor* Causer = nullptr, FDamageEvent DamEvent = FDamageEvent())
	{
		EffectTag = Tag;
		EffectDuration = Duration;
		EffectPower = Power;
		DamEve = DamEvent;
		EffectInstigator = Instigator;
		EffectCauser = Causer;
	}
};

USTRUCT(BlueprintType)
struct FPlayerBaseStats
{
	GENERATED_BODY()

	UPROPERTY()
	float Health;

	UPROPERTY()
	float WalkSpeed;

	UPROPERTY()
	float SprintSpeed;

	FPlayerBaseStats()
	{
		Health = 100.f;
		WalkSpeed = 320.f;
		SprintSpeed = 1200.f;
	}

	FPlayerBaseStats(float BaseHealth, float BaseWalkSpeed, float BaseSprintSpeed)
	{
		Health = BaseHealth;
		WalkSpeed = BaseWalkSpeed;
		SprintSpeed = BaseSprintSpeed;
	}
};