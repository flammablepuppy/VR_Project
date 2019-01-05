// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "vrPlayer.generated.h"

class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class USphereComponent;
class AvrPickup;

UCLASS()
class VR_PROJECT_API AvrPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AvrPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
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

	// Basic Locomotion Functions
	UFUNCTION(Category = "Locomotion")
	void MoveForward(float Value);
	UFUNCTION(Category = "Locomotion")
	void MoveRight(float Value);
	UFUNCTION(Category = "Locomotion")
	void MouseLookPitch(float Value);
	UFUNCTION(Category = "Locomotion")
	void MouseLookYaw(float Value); 
	/*UFUNCTION(Category = "Locomotion")  TODO: Enable snap turns
	void SnapTurn();
	UFUNCTION(Category = "Locomotion")
	void SnapRight();
	UFUNCTION(Category = "Locomotion")
	void SnapLeft();*/

	UPROPERTY(EditDefaultsOnly, Category = "Controls")
	bool bMouseEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "vrFunction")
	void OffsetRoot();

	UPROPERTY(BlueprintReadWrite, Category = "vrParameters")
	float PlayerHeight = 1.78f;

	// Controller Function calls
	UFUNCTION(Category = "Left Controller Functions")
	void LeftGripPull();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftGripRelease();
	UFUNCTION(Category = "Left Controller Functions")
	void LeftTriggerHandle(float Value);
	//UFUNCTION(Category = "Left Controller Functions")
	//void LeftTriggerPull(float Value);
	//UFUNCTION(Category = "Left Controller Functions")
	//void LeftTriggerRelease();
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
	//UFUNCTION(Category = "Right Controller Functions")
	//void RightTriggerPull(float Value);
	//UFUNCTION(Category = "Right Controller Functions")
	//void RightTriggerRelease();
	UFUNCTION(Category = "Right Controller Functions")
	void RightTopPush();
	UFUNCTION(Category = "Right Controller Functions")
	void RightTopRelease();
	UFUNCTION(Category = "Right Controller Functions")
	void RightBottomPush();
	UFUNCTION(Category = "Right Controller Functions")
	void RightBottomRelease();

	// Call Execution Functions
	void ExecuteGrip(AvrPickup* &ScanObject, AvrPickup* &GrippedObjectPointer, UMotionControllerComponent* GrabbingMC);
	void ExecuteDrop(AvrPickup* &ObjectToDrop);
	void ScanForClosestObject(USphereComponent* VolumeToScan, AvrPickup* &ScanRef, UMotionControllerComponent* MotionController);

	void BeginGrabHighlight(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	void EndGrabHighlight(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	AvrPickup* LeftScanTarget;
	UPROPERTY()
	AvrPickup* LeftHeldObject;
	UPROPERTY()
	AvrPickup* RightScanTarget;
	UPROPERTY()
	AvrPickup* RightHeldObject;

	/*UFUNCTION(Category = "Motion Controller Execution")
	void TriggerPulled(AvrPickup* HeldObject);
	UFUNCTION(Category = "Motion Controller Execution")
	void TriggerReleased(AvrPickup* HeldObject);*/


public:	

	UFUNCTION()
	FORCEINLINE AvrPickup* GetLeftHeldObject() { return LeftHeldObject; }
	UFUNCTION()
	FORCEINLINE AvrPickup* GetRightHeldObject() { return RightHeldObject; }

};
