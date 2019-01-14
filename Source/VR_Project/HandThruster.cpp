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

	if (bThrottleLocked && CurrentFuel > 0.f)
	{
		ApplyThrust(LockedThrottleValue);
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

	if (!bThrottleLocked) { CurrentTriggerAxisValue = Value; }

	if (Value > 0.1f && !bThrottleLocked && CurrentFuel > 0.f)
	{
		ApplyThrust(Value);
	}
	else if (bSetLockedThrottle)
	{
		LockedThrottleValue = Value;
		bSetLockedThrottle = false;
		CurrentTriggerAxisValue = LockedThrottleValue;
	}
}
void AHandThruster::TopPushed()
{
	Super::TopPushed();

}
void AHandThruster::TopReleased()
{
	Super::TopReleased();

}
void AHandThruster::BottomPushed()
{
	Super::BottomPushed();

	bThrottleLocked ? bThrottleLocked = false : bThrottleLocked = true;
	bSetLockedThrottle = true;
}
void AHandThruster::BottomReleased()
{
	Super::BottomReleased();

}

void AHandThruster::ApplyThrust(float ThrustAmount)
{
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());

	// Initialize Thrust
	FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * ThrustAmount;

	// Reduce Fuel
	CurrentFuel -= ThrustAmount * DeltaSeconds;

	// Ground Effect
	FHitResult TraceHit;
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, PickupMesh->GetComponentLocation(), PickupMesh->GetComponentLocation() + -PickupMesh->GetUpVector() * GroundEffectDistance, ECC_Visibility))
	{
		ThrusterOutput *= GroundEffectMultiplier;
	}

	// Translational Lift
	if (OwningPlayer->GetVelocity().Size() > TranslationalLiftSpeed)
	{
		ThrusterOutput *= TranslationalLiftMultiplier;
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
