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

	// Fuel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float MaxFuel = 10.f;
	UPROPERTY(BlueprintReadOnly, Category = "Thruster Properties")
	float CurrentFuel;

	// Thrust
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Thruster Properties")
	float ThrustPowerSetter = 16.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TerminalVelocitySpeed = 6100.f;

	// Ground Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectMultiplier = 0.32;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectLoss = 750.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectFull = 300.f;

	// Translational Lift
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TranslationalLiftMultiplier = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float MaxBenefitSpeed = 3250.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float BenefitDelta = 2750.f;
	UPROPERTY() // Set in BeginPlay, calculates the appropriate float for use in expo onset curve
	float TranslationalLiftCurveBase = 0.f;

	// Features
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float AutoHoverSlow = 0.54;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float AutoHoverFast = 0.45;
	UPROPERTY(BlueprintReadOnly)
	bool bThrottleLocked = false;
	UPROPERTY(BlueprintReadOnly)
	float LockedThrottleValue = 0.f;
	UPROPERTY(BlueprintReadOnly)
	bool bSetLockedThrottle = false;
	UPROPERTY(BlueprintReadOnly)
	bool bSetAutoHoverThrottle = false;
	UPROPERTY(BlueprintReadOnly)
	float CurrentTriggerAxisValue;

	UPROPERTY(BlueprintReadOnly)
	float DisplayNumber1;
	UPROPERTY(BlueprintReadOnly)
	float DisplayNumber2;


public:
	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPulled(float Value) override;
	virtual void TopPushed() override;
	virtual void TopReleased() override;
	virtual void BottomPushed() override;
	virtual void BottomReleased() override;
	virtual void Drop() override;

	UFUNCTION(BlueprintCallable)
	void ApplyThrust(float ThrustPercent);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetMaxFuel() { return MaxFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetCurrentFuel() { return CurrentFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetThrottleLocked() { return bThrottleLocked; }

};
