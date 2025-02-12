// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "vrBelt.generated.h"

/** 
* This component is attached to the collision capsule of the vrPlayer 
*  to provide a place to attach things to the player, like holsters,
*  mag carriers or whatever else I come up with 
*/

class AvrHolster;
class AvrPickup;
class AvrPlayer;
class UCameraComponent;
class UCapsuleComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VR_PROJECT_API UvrBelt : public USceneComponent
{
	GENERATED_BODY()

public:	
	UvrBelt();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

// VARIABLES
//////////////

	UPROPERTY(BlueprintReadOnly)
	AvrPlayer* OwningPlayer;

	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* TrackedHeadset;

	UPROPERTY(BlueprintReadOnly)
	TArray<AvrHolster*> EquippedHolsters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holster Properties")
	int32 MaxHolsters = 4;

// FUNCTIONS
//////////////

	/** Resets and populates the array containing all the attached holsters */
	UFUNCTION(BlueprintCallable)
	void FindAllHolsters();

public:

// PUBLIC FUNCTIONS
//////////////////////

	/**
	*	Finds a vacant holster
	*	@param PickupRequestingHolster, Finds a holster that is specifically for this item
	*	@param OverrideProximityRequirement, When true item will snap to the holster even if outside it's sphere
	*/
	UFUNCTION()
	AvrHolster* GetVacantHolster(AvrPickup* PickupRequestingHolster, bool OverrideProximityRequirement = false);

	// Drops all items in all holsters
	UFUNCTION()
	void YardSale(float NewLifespan = 1.5f, bool CanRecover = false);

	/** Populates output parameter array with all items found holstered on belt */
	UFUNCTION()
	void GetHolsteredItems(TArray<AvrPickup*>& Items);

	UFUNCTION(BlueprintCallable)
	void SpawnHolster(FVector BeltPosition, TSubclassOf<AvrHolster> HolsterType, AvrHolster*& OutHolster, bool RequiresProximity = false);

	UFUNCTION(BlueprintCallable)
	void SetMaxHolsters(int32 NewMax);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AvrHolster*> GetEquippedHolsters() { return EquippedHolsters; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetMaxHolsters() { return MaxHolsters; }

	UFUNCTION(BlueprintCallable)
	void DestoryHolsters();
	
};
