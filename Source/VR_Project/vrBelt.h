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

// FUNCTIONS
//////////////

	UFUNCTION()
	void FindAllHolsters();

	// TODO: Make a spawn holster function
	// TODO: Make an int32 variable to limit number of holsters

public:

// PUBLIC FUNCTIONS
//////////////////////

	UFUNCTION()
	AvrHolster* GetVacantHolster(AvrPickup* PickupRequestingHolster, bool OverrideProximityRequirement = false);


	/** Populates array with all items found holstered on belt */
	UFUNCTION()
	void GetHolsteredItems(TArray<AvrPickup*>& Items);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AvrHolster*> GetEquippedHolsters() { return EquippedHolsters; }

};
