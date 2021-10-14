// Copyright Aviator Games 2019

#include "HandThruster.h"

#include "vrPlayer.h"
#include "vrPickup.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

AHandThruster::AHandThruster()
{
	FuelRechargeSound = CreateDefaultSubobject<UAudioComponent>("Recharge Sound");
	FuelRechargeSound->SetupAttachment(RootComponent);
	FuelRechargeSound->VolumeMultiplier = 0.f;

	ThrusterSound = CreateDefaultSubobject<UAudioComponent>("Thruster Sound");
	ThrusterSound->SetupAttachment(RootComponent);
}
void AHandThruster::BeginPlay()
{
	Super::BeginPlay();

	CurrentFuel = MaxFuel;
	TranslationalLiftCurveBase = -1 / (BenefitDelta * BenefitDelta);
	CurrentTriggerAxisValue = 0.f;
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
			AutoHoverThrust = AutoHoverTargetThrust / (ThrustPowerSetter * (1.f + TranlationalLiftAdvantage));
		}
		else
		{
			AutoHoverThrust = AutoHoverTargetThrust / (ThrustPowerSetter * (1.f + GroundEffectMultiplier));
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
		// This bit makes the recharge rate double when on the ground
		if (OwningPlayer && !OwningPlayer->GetMovementComponent()->IsFalling())
		{
				CurrentFuel = FMath::Clamp(CurrentFuel + (MaxFuel / FuelRechargeRate) * DeltaTime, 0.f, MaxFuel);
		}

		CurrentFuel = FMath::Clamp(CurrentFuel + (MaxFuel / FuelRechargeRate) * DeltaTime, 0.f, MaxFuel);

		if (CurrentFuel/MaxFuel > 0.25f) bIsLowFuel = false;
		if (CurrentFuel == MaxFuel) bFuelRechargeTick = false;
	}
	PlayThrusterSound();
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

	//AvrPlayer* OP = Cast<AvrPlayer>(GetOwner());
	//if (OP && !OwningPlayer->GetMovementComponent()->IsFalling()) OP->Jump();
	bSetAutoHoverThrottle = true;

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
void AHandThruster::AddFuel(float AmountToAdd, bool AmountIsPercent)
{
	CurrentFuel += MaxFuel * AmountToAdd;
	CurrentFuel = FMath::Clamp<float>(CurrentFuel, 0.f, MaxFuel);
}
void AHandThruster::PlayThrusterSound()
{
	// START SFX
	if (!bWasThrusting && CurrentTriggerAxisValue > 0.1f && CurrentFuel > 0.f)
	{
		bWasThrusting = true;
		UGameplayStatics::SpawnSoundAttached(ThrustStartSound, this->GetRootComponent());
	}
	
	// THRUST-LOOP SFX
	if (CurrentTriggerAxisValue > 0.1f && CurrentFuel > 0.f)
	{
		const float NewVolume = FMath::Lerp(0.8f, 1.25f, CurrentTriggerAxisValue);
		ThrusterSound->SetVolumeMultiplier(NewVolume);
		const float NewPitch = FMath::Lerp(0.2f, 0.95f, CurrentTriggerAxisValue);
		ThrusterSound->SetPitchMultiplier(NewPitch);
	}
	else if (CurrentTriggerAxisValue < SMALL_NUMBER || CurrentFuel < SMALL_NUMBER)
	{
		ThrusterSound->SetVolumeMultiplier(0.f);
		ThrusterSound->SetPitchMultiplier(0.f);
	}

	// STOP SFX
	if (bWasThrusting && CurrentTriggerAxisValue < 0.1f)
	{
		bWasThrusting = false;
		UGameplayStatics::SpawnSoundAttached(ThrustStopSound, this->GetRootComponent());
	}
	else if (CurrentFuel < SMALL_NUMBER && bWasThrusting)
	{
		bWasThrusting = false;
		UGameplayStatics::SpawnSoundAttached(ThrustStopSound, this->GetRootComponent());
		UGameplayStatics::SpawnSoundAttached(FuelEmptySound, this->GetRootComponent());
	}
	
	// LOW FUEL TONE
	if(!bFuelRechargeTick && !bIsLowFuel && CurrentFuel/MaxFuel <= 0.25f)
	{
		bIsLowFuel = true;
		UGameplayStatics::SpawnSoundAttached(LowFuelSound, this->GetRootComponent());
	}
	
	// RECHARGE ON HANDLED IN RECHARGE TOGGLE
	// RECHARGE OFF
	if (!bFuelRechargeTick)
	{
		bFuelRechargeTick = false;
		FuelRechargeSound->FadeOut(0.1f, 0.f);
	}
}
void AHandThruster::FuelRechargeToggle()
{
	bFuelRechargeTick = true;
	FuelRechargeSound->VolumeMultiplier = 0.55f;
	FuelRechargeSound->FadeIn(0.1f);
}
