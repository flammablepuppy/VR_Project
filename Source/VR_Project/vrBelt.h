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

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AvrHolster*> GetEquippedHolsters() { return EquippedHolsters; }

protected:

UPROPERTY(BlueprintReadOnly)
AvrPlayer* OwningPlayer;

UPROPERTY(BlueprintReadOnly)
UCameraComponent* TrackedHeadset;

UPROPERTY()
TArray<AvrHolster*> EquippedHolsters;

UFUNCTION()
void FindAllHolsters();

public:

UFUNCTION()
AvrHolster* GetVacantHolster(AvrPickup* PickupRequestingHolster);
			
};
