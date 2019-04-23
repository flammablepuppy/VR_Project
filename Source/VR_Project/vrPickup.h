// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "vrPickup.generated.h"

class UStaticMeshComponent;
class UMotionControllerComponent;
class USceneComponent;
class USkeletalMesh;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPickupDropped, AvrPickup*, DroppedPickup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPickupSnappedOn, AvrPickup*, PickupToEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPickupGrabbed, AvrPickup*, GrabbedPickup);

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

	/** If a held object is tossed at this velocity or higher, it won't automatically attach to a vacant holster */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float NoHolsterSpeed = 180.f;

	/** Will attach to a vacant holster when dropped automatically */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	bool bSeeksHolster = true;
	
	//		FUNCTIONS
	//

	UFUNCTION()
	void MoveTo(USceneComponent * TargetComponent, FName TargetSocket);

public:	
	//		PUBLIC FUNCTIONS
	//

	UFUNCTION(BlueprintCallable)
	virtual void SnapInitiate(USceneComponent* NewParentComponent, FName SocketName = NAME_None);

	UFUNCTION()
	virtual	void SnapOn();

	UFUNCTION(BlueprintCallable)
	virtual void Drop();

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
	//UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	//void BPDrop();

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE USceneComponent* GetSnapTarget() { return SnapTarget; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetPickupEnabled() { return bPickupEnabled; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetSeeksHolster() { return bSeeksHolster; }


	//		SET
	//

	UFUNCTION(BlueprintCallable)
	void SetPickupEnabled(bool NewState); 

	UFUNCTION(BlueprintCallable)
	void NullifySnapTarget();

	UFUNCTION(BlueprintCallable)
	void SetSeeksHolster(bool NewState);

	//		DELEGATE
	//

	UPROPERTY(BlueprintAssignable)
	FPickupDropped OnDrop;

	UPROPERTY(BlueprintAssignable)
	FPickupSnappedOn OnSnappedOn;

	UPROPERTY(BlueprintAssignable)
	FPickupGrabbed OnGrabbed;

};
