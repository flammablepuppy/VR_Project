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
	//virtual void Tick(float DeltaTime) override;

protected:

	//		COMPONENTS
	//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* HolsterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* HolsterSphere;

	//		VARIABLES
	//

	/** Allows overlapping, dropped vrPickups to attach to holster */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	bool bProximityAttachEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AvrPickup> CompatiblePickup;

	UPROPERTY(BlueprintReadOnly)
	AvrPickup* HolsteredItem;

	//		FUNCTIONS
	//

	/** Subscribes overlapping vrPickup to CatchDroppedPickup function */
	UFUNCTION()
	void SubscribeCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Un-subscribes vrPickups leaving overlap */
	UFUNCTION()
	void UnsubCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	UFUNCTION()
	FORCEINLINE AvrPickup* GetHolsteredItem() { return HolsteredItem; }

	UFUNCTION()
	FORCEINLINE TSubclassOf<AvrPickup> GetCompatiblePickup() { return CompatiblePickup; }

	UFUNCTION()
	void CatchDroppedPickup(AvrPickup* DroppedPickup);

	UFUNCTION()
	void EnableHolsteredItem(AvrPickup* PickupToEnable);

	UFUNCTION()
	void ClearHolsteredItem(AvrPickup* DroppedPickup);
};

