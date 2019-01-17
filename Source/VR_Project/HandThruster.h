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
	float MaxFuel = 8.f;
	UPROPERTY(BlueprintReadOnly, Category = "Thruster Properties")
	float CurrentFuel;

	// Thrust
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float ThrustPower = 17.5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TerminalVelocitySpeed = 6900.f;
	
	// Ground Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectMultiplier = 0.45;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GroundEffectDistance = 750.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float GEBeginFalloff = 0.5f;

	// Translational Lift
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TranslationalLiftMultiplier = 0.6f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TranslationalLiftSpeed = 3250.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float TranslationalLiftFalloff = 1250.f;

	// Features
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Thruster Properties")
	float AutoHoverThrottle = 0.45f;
	UPROPERTY(BlueprintReadOnly)
	bool bThrottleLocked = false;
	UPROPERTY(BlueprintReadOnly)
	float LockedThrottleValue;
	UPROPERTY(BlueprintReadOnly)
	bool bSetLockedThrottle = false;
	UPROPERTY(BlueprintReadOnly)
	float CurrentTriggerAxisValue;

	UPROPERTY(BlueprintReadOnly)
	float LateralSpeed;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPulled(float Value) override;
	virtual void TopPushed() override;
	virtual void TopReleased() override;
	virtual void BottomPushed() override;
	virtual void BottomReleased() override;
	virtual void Drop() override;

	UFUNCTION(BlueprintCallable)
	void ApplyThrust(float ThrustAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetMaxFuel() { return MaxFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetCurrentFuel() { return CurrentFuel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool GetThrottleLocked() { return bThrottleLocked; }

};
