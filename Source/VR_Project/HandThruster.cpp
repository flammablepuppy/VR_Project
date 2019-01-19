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
	TranslationalLiftCurveBase = -1 / (BenefitDelta * BenefitDelta);
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

void AHandThruster::ApplyThrust(float ThrustPercent)
{
	if (CurrentFuel <= 0.f) { return; }

	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());

	// Initialize Thrust Velocity
	FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * ThrustPercent;

	// Reduce Fuel
	CurrentFuel -= ThrustPercent * DeltaSeconds;

	// Ground Effect
	FHitResult TraceHit;
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, PickupMesh->GetComponentLocation(), PickupMesh->GetComponentLocation() + -PickupMesh->GetUpVector() * GroundEffectFull, ECC_WorldStatic))
	{
		float HeightAboveTerrain = (GetActorLocation() - TraceHit.Location).Size();

		// Interoplate over a distance where ground effect is applied between max benefit and none
		// TODO: Change interpolation to be exponential instead of linear
		if (HeightAboveTerrain < GroundEffectFull)
		{
			ThrusterOutput *= (1.f + GroundEffectMultiplier);
		}
		else if (HeightAboveTerrain > GroundEffectFull && HeightAboveTerrain < GroundEffectLoss)
		{
			float Advantage = (HeightAboveTerrain - GroundEffectLoss) / (GroundEffectFull - GroundEffectLoss);
			Advantage *= GroundEffectMultiplier;
			ThrusterOutput *= (1.f + Advantage);
		}
	}

	// Translational Lift
	float PlayerLateralSpeed = FVector(OwningPlayer->GetVelocity().X, OwningPlayer->GetVelocity().Y, 0.f).Size();

	if (PlayerLateralSpeed > MaxBenefitSpeed - BenefitDelta && PlayerLateralSpeed < MaxBenefitSpeed + BenefitDelta)
	{
		float ForSquare = (PlayerLateralSpeed - MaxBenefitSpeed);
		float Advantage = 1.f + TranslationalLiftCurveBase * FMath::Square(ForSquare + 1);
		Advantage *= TranslationalLiftMultiplier;
		ThrusterOutput *= (1.f + Advantage);
	}
	else
	{
		DisplayNumber2 = 0.f;
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

		// Reduce velocity if over terminal velocity
		if (OwningPlayer->GetMovementComponent()->Velocity.Size() > TerminalVelocitySpeed)
		{
			OwningPlayer->GetMovementComponent()->Velocity *= 0.9f;
			OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
		}
	}
}
