// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "vrPickup.generated.h"

class UStaticMeshComponent;
class UMotionControllerComponent;

UCLASS()
class VR_PROJECT_API AvrPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AvrPickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent* OwningMC;

	UPROPERTY()
	bool bPickupEnabled = true;
	UPROPERTY()
	bool bMoving = false;
	UPROPERTY()
	bool bReadyToUse = false;
	UFUNCTION()
	void MoveToGrabbingMC();

	// Grabbing: Either uses Homing or Gravity snapping
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	bool bUsingGravitySnap = false;
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float HomingAcceleration = 20.f;
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float CurrentHomingSpeed = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Interaction") 
	float AttachAcceleration = 98.1f;
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector OldVelocity;
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TerminalVelocityFactor = 0.975f;
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TimeToRotate = 0.15f;
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float AttachThresholdDistance = 15.f;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPTriggerPull(float Value);
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPTopPush();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPTopRelease();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPBottomPush();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPBottomRelease();

	UFUNCTION()
	void SnapTo(UMotionControllerComponent* GrabbingController);
	UFUNCTION()
	void Drop();

	UFUNCTION()
	virtual void TriggerPulled(float Value);
	UFUNCTION()
	virtual void TopPushed();
	UFUNCTION()
	virtual void TopReleased();
	UFUNCTION()
	virtual void BottomPushed();
	UFUNCTION()
	virtual void BottomReleased();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UStaticMeshComponent* GetPickupMesh() { return PickupMesh; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetPickupEnabled() { return bPickupEnabled; }
	UFUNCTION(BlueprintCallable)
	void SetPickupEnabled(bool NewState); 
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UMotionControllerComponent* GetOwningMC() { return OwningMC; }

};
