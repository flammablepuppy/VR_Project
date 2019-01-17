// Copyright Aviator Games 2019

#include "HandThruster.h"
#include "vrPlayer.h"
#include "vrPickup.h"
#include "Engine/World.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

AHandThruster::AHandThruster()
{

}
void AHandThruster::BeginPlay()
{
	Super::BeginPlay();

	CurrentFuel = MaxFuel;
}
void AHandThruster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bThrottleLocked)
	{
		ApplyThrust(CurrentTriggerAxisValue);
	}
}

// Object functions
void AHandThruster::Drop()
{
	Super::Drop();

	bThrottleLocked = false;
}
void AHandThruster::TriggerPulled(float Value)
{
	Super::TriggerPulled(Value);

	if (bSetLockedThrottle)
	{
		LockedThrottleValue = Value;
		bSetLockedThrottle = false;
		CurrentTriggerAxisValue = LockedThrottleValue;
	}

	if (!bThrottleLocked && Value < 0.1f)
	{
		CurrentTriggerAxisValue = 0.f;
	}
	if (!bThrottleLocked && Value > 0.1f)
	{
		CurrentTriggerAxisValue = Value;
		ApplyThrust(Value);
	}
	else if (bThrottleLocked && Value > LockedThrottleValue)
	{
		CurrentTriggerAxisValue = Value;
	}
	else if (bThrottleLocked && Value <= LockedThrottleValue)
	{
		CurrentTriggerAxisValue = LockedThrottleValue;
	}

}
void AHandThruster::TopPushed()
{
	Super::TopPushed();

	LockedThrottleValue = AutoHoverThrottle;
	bThrottleLocked = true;
}
void AHandThruster::TopReleased()
{
	Super::TopReleased();

}
void AHandThruster::BottomPushed()
{
	Super::BottomPushed();

	CurrentTriggerAxisValue = 0.f;
	bThrottleLocked ? bThrottleLocked = false : bThrottleLocked = true;
	bSetLockedThrottle = true; // This bool is only used in TriggerPulled
}
void AHandThruster::BottomReleased()
{
	Super::BottomReleased();

}

void AHandThruster::ApplyThrust(float ThrustAmount)
{
	if (CurrentFuel <= 0.f) { return; }

	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());

	// Initialize Thrust Velocity
	FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * ThrustAmount;

	// Reduce Fuel
	CurrentFuel -= ThrustAmount * DeltaSeconds;

	// Ground Effect
	FHitResult TraceHit;
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, PickupMesh->GetComponentLocation(), PickupMesh->GetComponentLocation() + -PickupMesh->GetUpVector() * GroundEffectDistance, ECC_Visibility))
	{
		float HeightAboveTerrain = (GetActorLocation() - TraceHit.Location).Size();

		// Interoplate over a distance where ground effect is applied between max benefit and none
		// TODO: Change interpolation to be exponential instead of linear
		float BotInterp = GroundEffectDistance * GEBeginFalloff;
		float TopInterp = GroundEffectDistance;
		 
		if (HeightAboveTerrain < BotInterp)
		{
			ThrusterOutput *= (GroundEffectMultiplier + 1.f);
		}
		else if (HeightAboveTerrain > BotInterp && HeightAboveTerrain < TopInterp)
		{
			float GEInterp = (HeightAboveTerrain - TopInterp) / (BotInterp - TopInterp);
			GEInterp *= GroundEffectMultiplier;
			GEInterp += 1.f;
			ThrusterOutput *= GEInterp;
		}
	}

	// Translational Lift
	
	auto PlayerLatSpd = OwningPlayer->GetVelocity();
	PlayerLatSpd.Z = 0.f;

	LateralSpeed = PlayerLatSpd.Size();

	// Interpolate tranlational lift benefit between MaxEndurance, minimum and maximum TL airspeeds
	if (LateralSpeed > TranslationalLiftSpeed - TranslationalLiftFalloff && LateralSpeed < TranslationalLiftSpeed + TranslationalLiftFalloff)
	{
		float BenefitPercent = ((TranslationalLiftSpeed - TranslationalLiftFalloff) - FMath::Abs(TranslationalLiftSpeed - LateralSpeed)) / (TranslationalLiftSpeed - TranslationalLiftFalloff);
		BenefitPercent *= TranslationalLiftMultiplier;
		BenefitPercent += 1.f;
		ThrusterOutput *= BenefitPercent;
	}

	// Apply Thrust
	if (!OwningPlayer->GetMovementComponent()->IsFalling())
	{
		OwningPlayer->LaunchCharacter(ThrusterOutput, false, false);
	}
	if (OwningPlayer->GetMovementComponent()->IsFalling())
	{
		OwningPlayer->GetMovementComponent()->Velocity += ThrusterOutput;
		OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
		if (OwningPlayer->GetMovementComponent()->Velocity.Size() > TerminalVelocitySpeed)
		{
			OwningPlayer->GetMovementComponent()->Velocity *= 0.9f;
			OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
		}
	}
}
