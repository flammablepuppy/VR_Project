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
		CurrentFuel -= LockedThrottleValue * DeltaTime;
		AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());
		FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * LockedThrottleValue;

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
	float DeltaSeconds = GetWorld()->GetDeltaSeconds();

	if (Value > 0.1f && !bThrottleLocked && CurrentFuel > 0.f)
	{
		// TODO: Write a linetrace to check for ground effect thrust bonus application
		CurrentFuel -= Value * DeltaSeconds;
		AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());
		FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * Value;

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

// Info Functions
bool AHandThruster::CheckInGroundEffect()
{
	// TODO: Line trace straight down from bottom of cube, if world is in range return true
	return false;
}
