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
	/** Seconds of full power thrust on a full fuel tank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thruster Fuel") 
	float MaxFuel = 12.f;
	/** Whether or not the fuel automatically begins recharging, like Halo shields */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thruster Fuel") 
	bool bFuelRecharges = true;
	/** Seconds without thrust input before fuel begins recharging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thruster Fuel") 
	float FuelRechargeDelay = 2.5;
	/** How many seconds to go from empty to full */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thruster Fuel")
	float FuelRechargeRate = 12.f;

	FTimerHandle FuelRecharge_Handle;
	UPROPERTY(BlueprintReadOnly)
	float CurrentFuel;
	UPROPERTY()
	bool bFuelRechargeTick = false;
	UFUNCTION()
	void FuelRechargeToggle();

	// Thrust
	/** Power provided by a full trigger pull */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thruster Power") 
	float ThrustPowerSetter = 18.f;
	/** Max acheivable speed from thruster */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Power")
	float TerminalVelocitySpeed = 6100.f;

	// Ground Effect -- This behaves pretty unrealistically right now
	/** Percent increase in thruster power when in ground effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Ground Effect") 
	float GroundEffectMultiplier = 0.32;
	/** Max height at which ground effect fully tapers off */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Ground Effect") 
	float GroundEffectLoss = 600.f;
	/** Height below which ground effect is fully applied */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Ground Effect")
	float GroundEffectFull = 300.f;

	// Translational Lift
	/** When enabled, simulates a rotor which produces additional lift as relative wind (lateral velocity) increases */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Translational Lift") 
	bool bExperiencesTranslationalLift = true;
	/** Max additional lift percentage that can be gained by translational lift */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Translational Lift")
	float TranslationalLiftMultiplier = 0.95f;
	/** Speed at which TL advantage is highest, ie. Max Endurance Airspeed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Translational Lift")
	float MaxBenefitSpeed = 4000.f;
	/** TL benefit begins at MaxBenefitSpeed +/- this amount */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Translational Lift")
	float BenefitDelta = 3350.f;

	UPROPERTY()
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

	/** Target thrust value auto-hover calculates for taking into account GE and TL */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Features")
	float AutoHoverTargetThrust = 14.f;


	UPROPERTY(BlueprintReadOnly)
	float DisplayNumber1;
	UPROPERTY(BlueprintReadOnly)
	float DisplayNumber2;


public:
	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPulled(float Value) override;
	virtual void TopPushed() override;
	//virtual void TopReleased() override;
	//virtual void BottomPushed() override;
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
