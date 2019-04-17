// Copyright Aviator Games 2019

#include "Turret.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "HealthStats.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "vrPlayer.h"
#include "Camera/CameraComponent.h"
#include "vrProjectile.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	Base = CreateDefaultSubobject<UStaticMeshComponent>("Turret Base Mesh");
	RootComponent = Base;

	Turret = CreateDefaultSubobject<UStaticMeshComponent>("Turret Azimuth Mesh");
	Turret->SetupAttachment(RootComponent);

	BarrelPitchPoint = CreateDefaultSubobject<USceneComponent>("Barrel Pitch Point");
	BarrelPitchPoint->SetupAttachment(Turret);

	Barrel = CreateDefaultSubobject<UStaticMeshComponent>("Barrel Mesh");
	Barrel->SetupAttachment(BarrelPitchPoint);

	RadarZone = CreateDefaultSubobject<USphereComponent>("Radar SphereComp");
	RadarZone->SetupAttachment(RootComponent);
	RadarZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	RadarZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TurretHealth = CreateDefaultSubobject<UHealthStats>("Health Component");

}
void ATurret::BeginPlay()
{
	Super::BeginPlay();

	RadarZone->OnComponentBeginOverlap.AddDynamic(this, &ATurret::ScanTripWire);
	ScanForPawns();

	// Initialize AmmoSpeed for use in targeting prediction
	AvrProjectile* Ammo = Ammunition->GetDefaultObject<AvrProjectile>();
	AmmoSpeed = Ammo->GetProjectileSpeed();
}
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTargetDetectedInLOS)
	{
		TakeAim(FindFiringSolution());
	}
}

// Turret Aiming Functions
void ATurret::ScanTripWire(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	GetWorldTimerManager().SetTimer(ScanInterval_Timer, this, &ATurret::ScanForPawns, ScanSpeed, true);
}
void ATurret::ScanForPawns()
{
	// Scan whole RadarZone
	TArray<AActor*> FoundActors;
	TSubclassOf<APawn> PawnFilter;
	RadarZone->GetOverlappingActors(FoundActors, PawnFilter);
	for (AActor* Pawn : FoundActors)
	{
		APawn* FoundPawn = Cast<APawn>(Pawn);
		if (FoundPawn)
		{
			UHealthStats* PawnHealth = Cast<UHealthStats>(FoundPawn->GetComponentByClass(UHealthStats::StaticClass()));  
			if (PawnHealth && !PawnHealth->GetIsDead() && LineTraceForPawn(FoundPawn))  // Check to see if tested pawn is dead and if in LOS
			{
				if (!ClosestPawn) // ClosestPawn only gets assigned if the FoundPawn is also in LOS
				{
					ClosestPawn = FoundPawn;
					bTargetDetectedInLOS = true;
				}
				else // If multiple pawns are in LOS, the closer one is chosen
				{
					float FoundPawnDistance = (Base->GetComponentLocation() - FoundPawn->GetActorLocation()).Size();
					float ClosestPawnDistance = (Base->GetComponentLocation() - ClosestPawn->GetActorLocation()).Size();
					if (FoundPawnDistance < ClosestPawnDistance)
					{
						ClosestPawn = FoundPawn;
						bTargetDetectedInLOS = true;
					}
				}
			}
			else
			{
				ClosestPawn = nullptr;
				bTargetDetectedInLOS = false;
			}

			// To ensure scan turns on when pawn starts inside scan zone, called in BeginPlay and loop gets started here
			if (!GetWorldTimerManager().IsTimerActive(ScanInterval_Timer))
			{
				GetWorldTimerManager().SetTimer(ScanInterval_Timer, this, &ATurret::ScanForPawns, ScanSpeed, true);
			}
		}
	}
	// If there are no pawns in the radar zone, stop scanning
	if (FoundActors.Num() == 0)
	{
		ClosestPawn = nullptr;
		bTargetDetectedInLOS = false;
		GetWorldTimerManager().ClearTimer(ScanInterval_Timer);
	}
}
void ATurret::TakeAim(FVector Aimpoint)
{
	if (!bTargetDetectedInLOS) { return; }
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	FRotator StartRotation = BarrelPitchPoint->GetForwardVector().Rotation();
	FRotator RotationToHit = Aimpoint.Rotation();
	FRotator PreNormalDelta = RotationToHit - StartRotation;
	FRotator DeltaRotation = PreNormalDelta.GetNormalized(); // Normalizing makes it so it will freely cross the 0 degree local rotation instead of taking the long way around

	float ClampedYaw = FMath::Clamp(DeltaRotation.Yaw, -TurretYawSpeed * DeltaTime, TurretYawSpeed * DeltaTime);
	float ClampedPitch = FMath::Clamp(DeltaRotation.Pitch, -BarrelPitchSpeed * DeltaTime, BarrelPitchSpeed * DeltaTime);

	float NewYaw = StartRotation.Yaw + ClampedYaw; 
	float NewPitch = FMath::Clamp(StartRotation.Pitch + ClampedPitch, -10.f, 70.f); // Set min/max barrel pitch angle.

	Turret->SetWorldRotation(FRotator(0.f, NewYaw, 0.f));
	BarrelPitchPoint->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));

	if ((RotationToHit - StartRotation).IsNearlyZero(0.5f))
	{
		OpenFire();
	}
}
void ATurret::OpenFire()
{
	if (!GetWorldTimerManager().IsTimerActive(FireRate_Timer) && ClosestPawn)
	{
		BP_PlayFireFX();
		GetWorld()->SpawnActor<AvrProjectile>(Ammunition, Barrel->GetSocketTransform("Muzzle"));
		GetWorldTimerManager().SetTimer(FireRate_Timer, FireRate, false);
		ScanForPawns();
	}
}
FVector ATurret::FindFiringSolution()
{
	if (!ClosestPawn) { return Barrel->GetForwardVector(); }

	// Find where the player will be based on it's velocity in the time a shell takes to get to it, this will be used as an aim point if the SuggestVelocity fails
	AvrPlayer* ClosestvrPawn = Cast<AvrPlayer>(ClosestPawn);
	if (ClosestvrPawn)
	{
		PredictedPlayerPosition = 
			ClosestvrPawn->GetHeadsetCam()->GetComponentLocation() + // Use headset position ideally
			ClosestvrPawn->GetVelocity() *
			(ClosestvrPawn->GetVelocity().Normalize() *
			((ClosestvrPawn->GetActorLocation() - GetActorLocation()).Size() / AmmoSpeed));
	}
	else
	{
		PredictedPlayerPosition =
			ClosestPawn->GetActorLocation() +
			ClosestPawn->GetVelocity() *
			(ClosestPawn->GetVelocity().Normalize() *
			((ClosestPawn->GetActorLocation() - GetActorLocation()).Size() / AmmoSpeed));

		PredictedPlayerPosition += FVector(0.f, 0.f, (ClosestPawn->GetActorLocation() - GetActorLocation()).Size() / AmmoSpeed) * 490.f;
	}

	// Use engine parabola calculator
	FVector SuggestedVelocity = FVector::ZeroVector;
	if (UGameplayStatics::SuggestProjectileVelocity(this, SuggestedVelocity, Barrel->GetSocketLocation("Muzzle"), PredictedPlayerPosition, AmmoSpeed))
	{
		// For some reason this function will often return true with an absurd value that causes it to point straight up, do a simple check for this error and negate using it
		if (SuggestedVelocity.Z > 9900.f)
		{
			return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
			UE_LOG(LogTemp, Warning, TEXT("SuggestedVelocity: %s"), *SuggestedVelocity.ToString())

		}
		else if (SuggestedVelocity != FVector::ZeroVector)
		{
			return SuggestedVelocity;
			UE_LOG(LogTemp, Warning, TEXT("SuggestedVelocity: %s"), *SuggestedVelocity.ToString())
		}
		else
		{
			return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
			UE_LOG(LogTemp, Warning, TEXT("SuggestedVelocity: %s"), *SuggestedVelocity.ToString())

		}
	}
	else
	{
		//return Barrel->GetForwardVector();
		return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
		UE_LOG(LogTemp, Warning, TEXT("Suggestion is bad"))
	}
}
bool ATurret::LineTraceForPawn(APawn * TargetPawn)
{
	// Extend the trace to 5m beyong target location to ensure that it hits the pawn
	FVector TraceExtention = Barrel->GetSocketLocation("RadarScan") - TargetPawn->GetActorLocation();
	TraceExtention.GetSafeNormal();
	TraceExtention *= 500.f;

	FHitResult HitObject;
	GetWorld()->LineTraceSingleByChannel(HitObject, Barrel->GetSocketLocation("RadarScan"), TargetPawn->GetActorLocation() - TraceExtention, ECC_Camera);

	APawn* HitPawn = Cast<APawn>(HitObject.Actor);
	if (HitPawn)
	{
		return true;
	}
	else
	{
		return false;
	}
}
