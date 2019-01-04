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
	bool bMoving = false;
	UFUNCTION()
	void MoveToGrabbingMC();
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TimeToAttach = 0.25f;
	float CurrentTimeToAttach = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float AttachAcceleration = 1000.f;
	float CurrentAttachSpeed = 0.f;
	FVector OldVelocity; // initialize in BeginPlay
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TerminalVelocityFactor = 0.95f;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UStaticMeshComponent* GetPickupMesh() { return PickupMesh; }

	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPTriggerPull();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPTriggerRelease();
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
	void TriggerPulled();
	UFUNCTION()
	void TriggerReleased();
	/*UFUNCTION()
	void TopPushed();
	UFUNCTION()
	void TopReleased();
	UFUNCTION()
	void BottomPushed();
	UFUNCTION()
	void BottomReleased();*/

};
