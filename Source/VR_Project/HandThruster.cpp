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

	bThrottleLocked = true;
	LockedThrottleValue = AutoHoverThrottle;
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
	bSetLockedThrottle = true;
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
		float BotInterp, TopInterp, GEInterp;
		BotInterp = GroundEffectDistance * GEBeginFalloff;
		TopInterp = GroundEffectDistance;
		 
		if (HeightAboveTerrain < BotInterp)
		{
			ThrusterOutput *= (GroundEffectMultiplier);
		}
		else if (HeightAboveTerrain > BotInterp && HeightAboveTerrain < TopInterp)
		{
			GEInterp = (HeightAboveTerrain - TopInterp) / (BotInterp - TopInterp);
			ThrusterOutput *= (GroundEffectMultiplier * GEInterp);
		}
	}

	// Translational Lift
	float PlayerLateralSpeed = FMath::Abs(OwningPlayer->GetVelocity().X) + FMath::Abs(OwningPlayer->GetVelocity().Y);
	UE_LOG(LogTemp, Warning, TEXT("PlayerLaterSpeed: %f"), PlayerLateralSpeed)
		
	// TODO: Finish this. Interpolate tranlational lift benefit between MaxEndurance, minimum and maximum TL airspeeds
	if (PlayerLateralSpeed > TranslationalLiftSpeed - TranslationalLiftFalloff && PlayerLateralSpeed < TranslationalLiftSpeed + TranslationalLiftFalloff)
	{

		// UE_LOG(LogTemp, Warning, TEXT("Benefit: %f"), Benefit) LOG HOW MUCH TL BENEFIT IS BEING GIVEN
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
