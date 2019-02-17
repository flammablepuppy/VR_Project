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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // Seconds of full power thrust on a full fuel tank
	float MaxFuel = 7.f;
	UPROPERTY(BlueprintReadOnly, Category = "Thruster Properties")
	float CurrentFuel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thruster Properties")
	bool bFuelRecharges = true;
	FTimerHandle FuelRecharge_Handle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // Seconds without thrust input before fuel begins recharging
	float FuelRechargeDelay = 2.5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // How many seconds to go from empty to full
	float FuelRechargeRate = 12.f;
	UPROPERTY()
	bool bFuelRechargeTick = false;
	UFUNCTION()
	void FuelRechargeToggle();

	// Thrust
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thruster Properties") // Power provided by a full trigger pull
	float ThrustPowerSetter = 11.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TerminalVelocitySpeed = 6100.f;

	// Ground Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // Percent increase in thruster power when in ground effect
	float GroundEffectMultiplier = 0.85;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // Max height at which ground effect fully tapers off
	float GroundEffectLoss = 550.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties") // Height below which ground effect is fully applied
	float GroundEffectFull = 200.f;

	// Translational Lift
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TranslationalLiftMultiplier = 1.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float MaxBenefitSpeed = 4750.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float BenefitDelta = 4250.f;
	UPROPERTY() // Set in BeginPlay, calculates the appropriate float for use in expo onset curve
	float TranslationalLiftCurveBase = 0.f;
	UPROPERTY()
	float TranlationalLiftAdvantage = 0.f;

	// Features
	UPROPERTY() // Automatically gets set in Tick to a value that will allow for HIGE
	float AutoHoverThrust = 0.f;
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
