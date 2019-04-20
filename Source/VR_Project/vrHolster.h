// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "vrHolster.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AvrPickup;

UCLASS()
class VR_PROJECT_API AvrHolster : public AActor
{
	GENERATED_BODY()
	
public:	
	AvrHolster();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;

protected:

// COMPONENTS
///////////////

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* HolsterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* HolsterSphere;

// VARIABLES
//////////////

	/** Allows overlapping, dropped vrPickups to attach to holster */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	bool bProximityAttachEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AvrPickup> CompatiblePickup;

	UPROPERTY(BlueprintReadOnly)
	AvrPickup* HolsteredItem;

	UPROPERTY()
	bool bScanRunning = false;

// FUNCTIONS
//////////////

	/** Activates scan */
	UFUNCTION()
	void SubscribeCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Deactivates scan */
	UFUNCTION()
	void UnsubCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Subscribes closest overlapping vrPickup to CatchDroppedPickup function */
	UFUNCTION()
	void ScanForPickupsToCatch();

public:

// PUBLIC FUNCTION
////////////////////

	UFUNCTION()
	FORCEINLINE AvrPickup* GetHolsteredItem() { return HolsteredItem; }

	UFUNCTION()
	FORCEINLINE TSubclassOf<AvrPickup> GetCompatiblePickup() { return CompatiblePickup; }

	UFUNCTION()
	FORCEINLINE bool GetProximityAttachEnabled() { return bProximityAttachEnabled; }

	UFUNCTION(BlueprintCallable)
	void CatchDroppedPickup(AvrPickup* DroppedPickup);

	UFUNCTION()
	void EnableHolsteredItem(AvrPickup* PickupToEnable);

	UFUNCTION()
	void ClearHolsteredItem(AvrPickup* DroppedPickup);

	UFUNCTION(BlueprintCallable)
	void SetCompatiblePickup(TSubclassOf<AvrPickup> NewCompatiblePickup);

	UFUNCTION(BlueprintCallable)
	void SetProximityEnabled(bool NewState);
};

