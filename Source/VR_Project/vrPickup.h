// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "vrPickup.generated.h"

class UStaticMeshComponent;
class UMotionControllerComponent;
class USceneComponent;
class USkeletalMesh;

UCLASS()
class VR_PROJECT_API AvrPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AvrPickup();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	//		COMPONENTS
	//

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PickupMesh;

	//		MOVING AND ATTACHING VARIABLES
	//

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* SnapTarget;

	UPROPERTY()
	FName SnapSocket;

	UPROPERTY()
	UMotionControllerComponent* OwningMC;

	UPROPERTY()
	AvrPlayer* OwningPlayer;		

	/** Rate grabbed objets accelerate to grabbing hand */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float HomingAcceleration = 5.f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHomingSpeed = 0.f;

	/** Time it takes for grabbed object to match the rotation of the grabbing hand */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TimeToRotate = 0.1f;

	//		VARIABLES
	//

	/** Allows use of public snapping functions when true */
	UPROPERTY()
	bool bPickupEnabled = true;

	/** Activates movement through tick when true */
	UPROPERTY()
	bool bMoving = false;

	/** Allows input functions to fire when true */
	UPROPERTY()
	bool bReadyToUse = false;
	
	//		FUNCTIONS
	//

	UFUNCTION()
	void MoveTo(USceneComponent * TargetComponent, FName TargetSocket);

public:	
	//		PUBLIC FUNCTIONS
	//

	UFUNCTION()
	virtual void SnapInitiate(USceneComponent* NewParentComponent, FName SocketName = NAME_None);

	UFUNCTION()
	virtual	void SnapOn();

	UFUNCTION()
	virtual void Drop();

	//		PUBLIC VARIABLES -- TODO: Get rid of this, make it protected and make a getter.
	//

	UPROPERTY(BlueprintReadOnly)
	bool bTriggerPulled = false;

	//		BP INPUT CALLS
	//

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
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void BPDrop();

	//		INPUT CALLS
	//

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

	//		GET
	//

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UStaticMeshComponent* GetPickupMesh() { return PickupMesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UMotionControllerComponent* GetOwningMC() { return OwningMC; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE AvrPlayer* GetOwningPlayer() { return OwningPlayer; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetPickupEnabled() { return bPickupEnabled; }

	//		SET
	//

	UFUNCTION(BlueprintCallable)
	void SetPickupEnabled(bool NewState); 
};
