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
		CurrentFuel -= 0.333f * DeltaTime;
		AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());
		FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * 0.75f * TerminalThrustVelocity;

		if (!OwningPlayer->GetMovementComponent()->IsFalling())
		{
			OwningPlayer->LaunchCharacter(ThrusterOutput, false, false);
		}
		if (OwningPlayer->GetMovementComponent()->IsFalling())
		{
			OwningPlayer->GetMovementComponent()->Velocity += ThrusterOutput;
			OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
		}
	}
}

// Object functions
void AHandThruster::TriggerPulled(float Value)
{
	Super::TriggerPulled(Value);

	float DeltaSeconds = GetWorld()->GetDeltaSeconds();

	if (Value > 0.f && !bThrottleLocked && CurrentFuel > 0.f)
	{
		CurrentFuel -= Value * DeltaSeconds;
		AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwningMC()->GetOwner());
		FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * Value * TerminalThrustVelocity;

		if (!OwningPlayer->GetMovementComponent()->IsFalling()) 
		{
			OwningPlayer->LaunchCharacter(ThrusterOutput, false, false); 
		}
		if (OwningPlayer->GetMovementComponent()->IsFalling()) 
		{
			OwningPlayer->GetMovementComponent()->Velocity += ThrusterOutput;
			OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
		}
	}
}
void AHandThruster::TopPushed()
{
}
void AHandThruster::TopReleased()
{
}
void AHandThruster::BottomPushed()
{
	bThrottleLocked ? bThrottleLocked = false : bThrottleLocked = true;
}
void AHandThruster::BottomReleased()
{
}

// Info Functions
bool AHandThruster::CheckInGroundEffect()
{
	// TODO: Line trace straight down from bottom of cube, if world is in range return true
	return false;
}
