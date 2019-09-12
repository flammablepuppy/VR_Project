// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "vrPlayer.generated.h"

class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class USphereComponent;
class AvrPickup;
class UHealthStats;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGripRequest, UMotionControllerComponent*, RequestingController);

UCLASS()
class VR_PROJECT_API AvrPlayer : public ACharacter
{
	GENERATED_BODY()

public:

	AvrPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;

	// Components
	//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* vrRoot;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* HeadsetCamera;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMotionControllerComponent* LeftController;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMotionControllerComponent* RightController;
 	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* LeftVolume;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* RightVolume;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UHealthStats* HealthStatsComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UvrBelt* UtilityBelt;

	/** When true, clicking left stick will kill self */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Properties")
	bool bCommitsSeppuku = false;

	// Basic Locomotion Functions
	//

	UFUNCTION(Category = "Locomotion")
	void MoveForward(float Value);
	UFUNCTION(Category = "Locomotion")
	void MoveRight(float Value);
	UFUNCTION(Category = "Locomotion")
	void MouseLookPitch(float Value);
	UFUNCTION(Category = "Locomotion")
	void MouseLookYaw(float Value);
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	bool bMouseEnabled = true;
	UFUNCTION()
	void SmallJump();

	// Snap Turn
	//

	UFUNCTION(Category = "Locomotion")
	void SnapTurn(float Value);
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float SnapTurnIncrement = 90.f;
	UPROPERTY()
	bool bSnapTurnReady = true;

	// VR Specific Movement
	//

	UFUNCTION()
	void OffsetRoot();

	/** Height of player based on HMD position */
	UPROPERTY(BlueprintReadWrite, Category = "vrParameters")
	float PlayerHeight = 1.78f;

	// Motion Input
	//

	UFUNCTION(BlueprintCallable)
	void MotionInputScan();

	UPROPERTY(BlueprintReadOnly)
	FVector HeadRelative;
	UPROPERTY(BlueprintReadOnly)
	FVector LeftRelative;
	UPROPERTY(BlueprintReadOnly)
	FVector RightRelative;

	UPROPERTY(BlueprintReadOnly)
	FVector HeadRelVel = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	FVector LeftRelVel = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	FVector RightRelVel = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector HeadLastRelPos = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	FVector LeftLastRelPos = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	FVector RightLastRelPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector VelocityLastTick = FVector::ZeroVector;

	// Impact Damage
	UFUNCTION()
	void ApplyImpactDamage();

		UPROPERTY(EditDefaultsOnly, Category = "Motion Input: Impact Damage")
		USoundCue* ImpactDamageSound;

		/** Determines whether pawn can suffer impact damage */
		UPROPERTY()
		bool bImpactDamageActive = true;

		/** Velocity change threshold beyond which damage is applied to the player */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage")
		float VelocityChangeDamageSpeed = 1250.f; 

		/** The minimum amount of damage that will be dealt to the player after an abrupt velocity change */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage") 
		float MinimumImpactDamage = 15.f;

		/** Damage delt per cm/s over velocity change threshold */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage") 
		float ExponentialImpactDamage = 0.012f;

		UPROPERTY(EditDefaultsOnly, Category = "Motion Input: Impact Damage")
		TSubclassOf<UDamageType> MotionDamage;

	// Jump
	UFUNCTION()
	void MotionJump();

		/** Sound that plays when a small jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		USoundCue* SmallJumpSound;

		/** Upward impulse passed to the jump function when a small jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float SmallJumpHeight = 550.f;

		/** Sound that plays when a big jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		USoundCue* BigJumpSound;

		/** Upward impulse passed to the jump function when a big jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHeight = 840.f;

		/** The amount of vertical acceleration the HMD must hit */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHeadReq = 1.2f;

		/** The amount of vertical acceleration both the controllers must hit to trigger a jump */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHandReq = 8.f;

		/** Additional impulse in the direction you're looking when performing a Big Jump with forward movement input */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float PostJumpImpulse = 500.f;

		/** Percentage of velocity added as bonus to velocity */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float ChainJumpMultiplier = 0.05f;

		/** Percentage of MaxSprintSpeed that jump will fire without head movement */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHighSpeedFireMultiplier = 1.15f;

		FTimerHandle Leap_Timer;
		
	// Sprint
	UFUNCTION()
	void MotionSprint(float ImpulsePercent, FVector Direction);

		/** The X and Z relative velocity are added together and compared to this value, if they exceed it MotionSprint is fired */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMinImpulseSpeed = 14.f;

		/** The velocity that is required to pass a 1.0 multiplier to the calculation of how much impulse power is garnered from a sprint arm swing */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMaxImpulseSpeed = 30.f;

		/** Time that must elapse before a motion controller can trigger another sprint impulse */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintCooldownDuration = 0.2f;

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintDecelResetDuration = 0.75f;

		/** Default walking speed */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMinSpeed = 280.f;

		/** Max speed attainable by sprinting */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMaxSpeed = 1050.f;

		/** Friction when sprinting, low value better simulates momentum of running */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintingFriction = 0.18f;

		/** Normal friction value, applied when not sprinting */
		UPROPERTY()
		float SprintNormalFriction = 2.f;

		FTimerHandle SprintLeft_Timer;
		FTimerHandle SprintRight_Timer;
		FTimerHandle SprintReset_Timer;
		FTimerHandle SpawnJumpPrevention_Timer;

		/** Used for determining sprint impulse direction */
		FVector SprintLeftLastPos = FVector::ZeroVector;
		FVector SprintRightLastPos = FVector::ZeroVector;

	// Controller Function calls
	UFUNCTION(Category = "Left Controller Functions")
	void LeftGripPull();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftGripRelease();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftTriggerHandle(float Value);
	UFUNCTION(Category = "Left Controller Functions")
	void LeftTopPush();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftTopRelease();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftBottomPush();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftBottomRelease();

	UFUNCTION(Category = "Right Controller Functions")
	void RightGripPull();
	UFUNCTION(Category = "Right Controller Functions")
	void RightGripRelease();
	UFUNCTION(Category = "Right Controller Functions")
	void RightTriggerHandle(float Value);
	UFUNCTION(Category = "Right Controller Functions")
	void RightTopPush();
	UFUNCTION(Category = "Right Controller Functions")
	void RightTopRelease();
	UFUNCTION(Category = "Right Controller Functions")
	void RightBottomPush();
	UFUNCTION(Category = "Right Controller Functions")
	void RightBottomRelease();

	UFUNCTION()
	void ResetTestingMap();

	// Call Execution Functions
	void ExecuteGrip(AvrPickup* &ScanObject, AvrPickup* &GrippedObjectPointer, UMotionControllerComponent* GrabbingMC);
	void ExecuteDrop(AvrPickup* &ObjectToDrop);
	void ScanForClosestObject(USphereComponent* VolumeToScan, AvrPickup* &ScanRef, UMotionControllerComponent* MotionController);

	UFUNCTION()
	void BeginGrabHighlight(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void EndGrabHighlight(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	AvrPickup* LeftScanTarget;
	UPROPERTY()
	AvrPickup* LeftHeldObject;
	UPROPERTY()
	AvrPickup* RightScanTarget;
	UPROPERTY()
	AvrPickup* RightHeldObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Level Reset")
	FName LevelToLoad = "Lobby";

public:	

	UFUNCTION()
	FORCEINLINE AvrPickup* GetLeftHeldObject() { return LeftHeldObject; }

	UFUNCTION()
	FORCEINLINE AvrPickup* GetRightHeldObject() { return RightHeldObject; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetMouseEnabled() { return bMouseEnabled; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UMotionControllerComponent* GetLeftMC() { return LeftController; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UMotionControllerComponent* GetRightMC() { return RightController; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UCameraComponent* GetHeadsetCam() { return HeadsetCamera; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UvrBelt* GetUtilityBelt() { return UtilityBelt; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UHealthStats* GetHealthStats() { return HealthStatsComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetSprintSpeed() { return SprintMaxSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetBigJumpHighSpeedFireMultiplier() { return BigJumpHighSpeedFireMultiplier; }

	UFUNCTION(BlueprintCallable)
	void SetMouseEnabled(bool NewState);

	UFUNCTION()
	FORCEINLINE FVector GetVelocityLastTick() { return VelocityLastTick; };

	UFUNCTION(BlueprintCallable)
	void SetImpactDamageEnabled(bool NewState);

	//		FOR DEBUG DISPLAY ON HUD
	//

	UFUNCTION(BlueprintImplementableEvent)
	void BPDebugDisplay();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SendCombatText(FVector DisplayLocation, float Damage, FVector FiringPlayerPosition);

	UFUNCTION()
	FORCEINLINE FVector GetHeadRelVel() { return HeadRelVel; }

	UFUNCTION()
	FORCEINLINE FVector GetLeftRelVel() { return LeftRelVel; }

	UFUNCTION()
	FORCEINLINE FVector GetRightRelVel() { return RightRelVel; }

	UPROPERTY()
	float WhatsTheFriction = 2.f;

// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable)
	FGripRequest OnGrip;
};