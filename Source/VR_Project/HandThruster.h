// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "HandThruster.generated.h"

/**
 * 
 */
UCLASS()
class VR_PROJECT_API AHandThruster : public AvrPickup
{
	GENERATED_BODY()
	
public:
	AHandThruster();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float MaxFuel = 5.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Thruster Properties")
	float CurrentFuel;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float ThrustPower = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TerminalVelocitySpeed = 9000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectThrustMultiplier = 1.45f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Thruster Properties")
	bool bThrottleLocked = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Thruster Properties")
	float LockedThrottleValue;
	UFUNCTION()
	bool CheckInGroundEffect();
	bool bSetLockedThrottle = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Thruster Properties")
	float CurrentTriggerAxisValue;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPulled(float Value) override;
	virtual void TopPushed() override;
	virtual void TopReleased() override;
	virtual void BottomPushed() override;
	virtual void BottomReleased() override;
	virtual void Drop() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetMaxFuel() { return MaxFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetCurrentFuel() { return CurrentFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetThrottleLocked() { return bThrottleLocked; }

};
