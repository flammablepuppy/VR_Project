// Copyright Aviator Games 2019

#include "HandThruster.h"
#include "vrPlayer.h"
#include "vrPickup.h"
#include "Engine/World.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

AHandThruster::AHandThruster()
{
	
}
void AHandThruster::BeginPlay()
{
	Super::BeginPlay();

	CurrentFuel = MaxFuel;
	TranslationalLiftCurveBase = -1 / (BenefitDelta * BenefitDelta);
}
void AHandThruster::FuelRechargeToggle()
{
	bFuelRechargeTick = true;
}
void AHandThruster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bSetAutoHoverThrottle)
	{
		FVector OwnerVelocity = OwningPlayer->GetCharacterMovement()->Velocity;
		OwnerVelocity.Z = 0.f;
		if (OwnerVelocity.Size() > (MaxBenefitSpeed - BenefitDelta) &&
			OwnerVelocity.Size() < (MaxBenefitSpeed + BenefitDelta) &&
			bExperiencesTranslationalLift)
		{
			AutoHoverThrust = 11.25f / (ThrustPowerSetter * (1.f + TranlationalLiftAdvantage));
		}
		else
		{
			AutoHoverThrust = 10.95f / (ThrustPowerSetter * (1.f + GroundEffectMultiplier));
		}

		LockedThrottleValue = AutoHoverThrust;
		CurrentTriggerAxisValue = AutoHoverThrust;
		bSetAutoHoverThrottle = false;
		bThrottleLocked = true;
	}
	if (bThrottleLocked)
	{
		ApplyThrust(CurrentTriggerAxisValue);
	}
	if (bFuelRechargeTick && bFuelRecharges)
	{
		CurrentFuel = FMath::Clamp(CurrentFuel + (MaxFuel / FuelRechargeRate) * DeltaTime, 0.f, MaxFuel);
		if (CurrentFuel == MaxFuel)
		{
			bFuelRechargeTick = false;
		}
	}
}

// Object functions
void AHandThruster::Drop()
{
	Super::Drop();

	bThrottleLocked = false;
	CurrentTriggerAxisValue = 0.f;
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
		if (!GetWorldTimerManager().IsTimerActive(FuelRecharge_Handle)) { GetWorldTimerManager().SetTimer(FuelRecharge_Handle, this, &AHandThruster::FuelRechargeToggle, FuelRechargeDelay); }
	}
	if (!bThrottleLocked && Value > 0.1f)
	{
		if (GetWorldTimerManager().IsTimerActive(FuelRecharge_Handle)) { GetWorldTimerManager().ClearTimer(FuelRecharge_Handle); }

		bFuelRechargeTick = false;
		CurrentTriggerAxisValue = Value;
		ApplyThrust(Value);
	}
	else if (bThrottleLocked && Value > LockedThrottleValue)
	{
		if (GetWorldTimerManager().IsTimerActive(FuelRecharge_Handle)) { GetWorldTimerManager().ClearTimer(FuelRecharge_Handle); }

		bFuelRechargeTick = false;
		CurrentTriggerAxisValue = Value;
	}
	else if (bThrottleLocked && Value <= LockedThrottleValue)
	{
		if (GetWorldTimerManager().IsTimerActive(FuelRecharge_Handle)) { GetWorldTimerManager().ClearTimer(FuelRecharge_Handle); }

		bFuelRechargeTick = false;
		CurrentTriggerAxisValue = LockedThrottleValue;
	}

} 
void AHandThruster::TopPushed()
{
	Super::TopPushed();

	bSetAutoHoverThrottle = true;

}
void AHandThruster::TopReleased()
{
	Super::TopReleased();

}
void AHandThruster::BottomPushed()
{
	Super::BottomPushed();

}
void AHandThruster::BottomReleased()
{
	Super::BottomReleased();

	CurrentTriggerAxisValue = 0.f;
	bThrottleLocked ? bThrottleLocked = false : bThrottleLocked = true;
	bSetLockedThrottle = true; // This bool is only used in TriggerPulled
}

void AHandThruster::ApplyThrust(float ThrustPercent)
{
	// If fuel runs out, turn everything off
	if (CurrentFuel <= 0.f) 
	{ 
		if (bThrottleLocked)
		{
			bThrottleLocked = false;
			CurrentTriggerAxisValue = 0.f;
		}
		return; 
	}

	// Initialize ThrustPower
	float ThrustPower = ThrustPowerSetter;

	// Reduce Fuel
	CurrentFuel -= ThrustPercent * GetWorld()->GetDeltaSeconds();

	// Ground Effect
	FHitResult TraceHit;
	FCollisionResponseParams Params;
	Params.CollisionResponse.Pawn = ECR_Ignore; // This prevents player from hitting themselves with the trace and getting GE benefit from it
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, PickupMesh->GetComponentLocation(), PickupMesh->GetComponentLocation() + -PickupMesh->GetUpVector() * GroundEffectLoss, 
		ECC_WorldStatic, FCollisionQueryParams::DefaultQueryParam, Params))
	{
		float HeightAboveTerrain = (GetActorLocation() - TraceHit.Location).Size();

		// Interoplate over a distance where ground effect is applied between max benefit and none
		// Should switch this to an expo curve to make the cushioning come on more appropriately, though it feels pretty good now
		if (HeightAboveTerrain < GroundEffectFull)
		{
			ThrustPower *= (1.f + GroundEffectMultiplier);
		}
		else if (HeightAboveTerrain > GroundEffectFull)
		{
			float Advantage = (GroundEffectLoss - HeightAboveTerrain) / (GroundEffectLoss - GroundEffectFull);
			Advantage *= GroundEffectMultiplier;
			ThrustPower *= (1.f + Advantage);
			DisplayNumber2 = Advantage;
		}
	}

	// Translational Lift
	if (bExperiencesTranslationalLift)
	{
		float PlayerLateralSpeed = FVector(OwningPlayer->GetVelocity().X, OwningPlayer->GetVelocity().Y, 0.f).Size();
		if (PlayerLateralSpeed > MaxBenefitSpeed - BenefitDelta && PlayerLateralSpeed < MaxBenefitSpeed + BenefitDelta)
		{
			float ForSquare = (PlayerLateralSpeed - MaxBenefitSpeed);
			float Advantage = 1.f + TranslationalLiftCurveBase * FMath::Square(ForSquare + 1);
			Advantage *= TranslationalLiftMultiplier;
			ThrustPower *= (1.f + Advantage);

			TranlationalLiftAdvantage = Advantage; // Variable used in determining auto hover thrust
		}
	}

	// Initialize ThrusterOutput Vector with adjust power
	FVector ThrusterOutput = GetPickupMesh()->GetUpVector() * ThrustPower * ThrustPercent;
	DisplayNumber1 = ThrusterOutput.Size();

	// Apply Thrust
	if (!OwningPlayer->GetMovementComponent()->IsFalling())
	{
		OwningPlayer->LaunchCharacter(ThrusterOutput + FVector(0.f, 0.f, 10.f), false, false);
	}
	else
	{
		OwningPlayer->GetMovementComponent()->Velocity += ThrusterOutput;
	}

	// Enforce TerminalVelocitySpeed
	if (OwningPlayer->GetMovementComponent()->Velocity.Size() > TerminalVelocitySpeed)
	{
		OwningPlayer->GetMovementComponent()->Velocity *= TerminalVelocitySpeed / OwningPlayer->GetMovementComponent()->Velocity.Size();
	}

	OwningPlayer->GetMovementComponent()->UpdateComponentVelocity();
}