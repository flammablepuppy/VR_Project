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
	FVector HeadRelVel = HeadLastRelPos - HeadRelative;
	UPROPERTY(BlueprintReadOnly)
	FVector LeftRelVel = -(LeftLastRelPos - LeftRelative);
	UPROPERTY(BlueprintReadOnly)
	FVector RightRelVel = -(RightLastRelPos - RightRelative);

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

		/** Velocity change threshold beyond which damage is applied to the player */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage")
		float VelocityChangeDamageSpeed = 1000.f; 

		/** The minimum amount of damage that will be dealt to the player after an abrupt velocity change */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage") 
		float MinimumImpactDamage = 30.f;

		/** Damage delt per cm/s over velocity change threshold */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Impact Damage") 
		float ExponentialImpactDamage = 0.012f;

		UPROPERTY(EditDefaultsOnly, Category = "Motion Input: Impact Damage")
		TSubclassOf<UDamageType> MotionDamage;

	// Jump
	UFUNCTION()
	void MotionJump();

		/** Sound that plays when a big jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		USoundCue* BigJumpSound;

		/** The amount of vertical acceleration the HMD must hit to trigger a jump */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHeadReq = 1.5f;

		/** The amount of vertical acceleration both the controllers must hit to trigger a jump */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHandReq = 6.f;

		/** The minimum duration the jumping motion must be made to trigger a big jump */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float JumpDurationReq = 0.08f;

		/** Upward impulse passed to the jump function when a big jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float BigJumpHeight = 840.f;

		/** Additional impulse in the direction you're looking when performing a Big Jump with forward movement input */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float JumpBigForwardImpulse = 65.f;

		/** Sound that plays when a small jump fires */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		USoundCue* SmallJumpSound;

		/** When both hands move vertically by this speed without head movement upward, triggers a small jump */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float JumpSmallReq = 16.f;

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Jump")
		float SmallJumpHeight = 550.f;

		/** Set to true by MoveForward when axis value is greater than 0.3, used to limit accidental jumps and amplify forward motion on big jumps */
		UPROPERTY()
		bool bHasForwardMovementInput = false;

	// Sprint
	UFUNCTION()
	void MotionSprint(FVector ImpulseDirection, float ImpulsePercent);

		/** The X and Z relative velocity are added together and compared to this value, if they exceed it MotionSprint is fired */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMinImpulseSpeed = 14.f;

		/** The velocity that is required to pass a 1.0 multiplier to the calculation of how much impulse power is garnered from a sprint arm swing */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintMaxImpulseSpeed = 30.f;

		/** Time that must elapse before a motion controller can trigger another sprint impulse */
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintCooldownDuration = 0.3f;

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input: Sprint")
		float SprintDecelResetDuration = 0.75f;

		UPROPERTY()
		float SprintMinSpeed = 320.f;

		UPROPERTY()
		float SprintMaxSpeed = 1200.f;

		UPROPERTY()
		float SprintingFriction = 0.18f;

		UPROPERTY()
		float SprintNormalFriction = 2.f;

		FTimerHandle SprintLeft_Timer;
		FTimerHandle SprintRight_Timer;
		FTimerHandle SprintDecelReset_Timer;
		UPROPERTY()
		FVector SprintLeftLastPos = FVector::ZeroVector;
		UPROPERTY()
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

	UFUNCTION(BlueprintCallable)
	void SetMouseEnabled(bool NewState);

	UFUNCTION()
	FORCEINLINE FVector GetVelocityLastTick() { return VelocityLastTick; };

	//		FOR DEBUG DISPLAY ON HUD
	//

	UFUNCTION(BlueprintImplementableEvent)
	void BPDebugDisplay();

	UFUNCTION()
	FORCEINLINE FVector GetHeadRelVel() { return HeadRelVel; }

	UFUNCTION()
	FORCEINLINE FVector GetLeftRelVel() { return LeftRelVel; }

	UFUNCTION()
	FORCEINLINE FVector GetRightRelVel() { return RightRelVel; }

	UPROPERTY()
	float WhatsTheFriction = 2.f;
};
