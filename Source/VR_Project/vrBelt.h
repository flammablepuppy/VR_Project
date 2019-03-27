// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "vrBelt.generated.h"

/** This component is attached to the collision capsule of the vrPlayer to provide a place to attach things to the player, like holsters or mag carriers */

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

	UPROPERTY(BlueprintReadOnly)
	class AvrPlayer* OwningPlayer;

	UPROPERTY(BlueprintReadOnly)
	class UCameraComponent* TrackedHeadset;

		
};
