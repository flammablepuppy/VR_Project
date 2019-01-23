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

	// Basic Locomotion Functions
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
	UFUNCTION(Category = "Locomotion")
	void SnapTurn(float Value);
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float SnapTurnIncrement = 90.f;
	UPROPERTY()
	bool bSnapTurnReady = true;

	// VR Specific Movement
	UFUNCTION(BlueprintCallable, Category = "vrFunction")
	void OffsetRoot();
	UPROPERTY(BlueprintReadWrite, Category = "vrParameters")
	float PlayerHeight = 1.78f;

	UPROPERTY(BlueprintReadOnly, Category = "vrParameters")
	FVector VelocityLastTick = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, Category = "vrParameters")
	TSubclassOf<UDamageType> MotionDamage;

	UPROPERTY(BlueprintReadOnly, Category = "vrParameters")
	FVector HeadLastRelPos = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "vrParameters")
	FVector LeftLastRelPos = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "vrParameters")
	FVector RightLastRelPos = FVector::ZeroVector;

	// Motion Input
	UFUNCTION(BlueprintCallable)
	void MotionInputScan();
		// Impact Damage
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Locomotion")
		float VelocityChangeDamageSpeed = 1000.f; // Velocity change threshold beyond which damage is applied to the player
		UFUNCTION()
		void ApplyImpactDamage(float VelocityChange);
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input") // The minimum amount of damage that will be dealt to the player after an abrupt velocity change
		float MinimumImpactDamage = 15.f;
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input") // Damage delt per cm/s over velocity change threshold
		float ExponentialImpactDamage = 0.012f;
		// Jumping TODO: Finish jump implementation
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input")
		float HeadJumpRequiredZ = 1.5f;
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input")
		float LeftJumpRequiredZ = 2.5f;
		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion Input")
		float RightJumpRequiredZ = 2.5f;


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
	UFUNCTION(BlueprintCallable)
	void SetMouseEnabled(bool NewState);
	UFUNCTION()
	FORCEINLINE FVector GetVelocityLastTick() { return VelocityLastTick; };
};
