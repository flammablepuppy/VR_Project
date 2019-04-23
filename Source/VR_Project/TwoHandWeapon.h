// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "TwoHandWeapon.generated.h"

/**
 * 
 */
UCLASS()
class VR_PROJECT_API ATwoHandWeapon : public AvrPickup
{
	GENERATED_BODY()

protected:
	ATwoHandWeapon();
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
protected:
	   
// COMPONENTS
///////////////

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USphereComponent* AftGripSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* ForeGripSphere;

// VARIABLES
//////////////



	
};
