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

	// TODO: Figure out the best way to get the projectile initial speed so I don't have to hardcode values
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
			if (!ClosestPawn)
			{
				ClosestPawn = FoundPawn;
			}
			else
			{
				float FoundPawnDistance = (Base->GetComponentLocation() - FoundPawn->GetActorLocation()).Size();
				float ClosestPawnDistance = (Base->GetComponentLocation() - ClosestPawn->GetActorLocation()).Size();
				if (FoundPawnDistance < ClosestPawnDistance)
				{
					ClosestPawn = FoundPawn;
				}
			}
			
			// Check if we have LOS
			FHitResult HitObject;
			// Extend the trace to 5m beyong target location to ensure that it hits the pawn
			FVector TraceExtention = Barrel->GetSocketLocation("RadarScan") - ClosestPawn->GetActorLocation();
			TraceExtention.GetSafeNormal();
			TraceExtention *= 500.f;

			GetWorld()->LineTraceSingleByChannel(HitObject, Barrel->GetSocketLocation("RadarScan"), ClosestPawn->GetActorLocation() - TraceExtention, ECC_WorldStatic);

			APawn* HitPawn = Cast<APawn>(HitObject.Actor);
			if (HitPawn)
			{
				bTargetDetectedInLOS = true;
			}
			else
			{
				bTargetDetectedInLOS = false;
			}

			if (!GetWorldTimerManager().IsTimerActive(ScanInterval_Timer))
			{
				GetWorldTimerManager().SetTimer(ScanInterval_Timer, this, &ATurret::ScanForPawns, ScanSpeed, true);
			}
		}
	}
	// If there are no pawns in the radar zone, stop scanning
	if (FoundActors.Num() == 0)
	{
		bTargetDetectedInLOS = false;
		GetWorldTimerManager().ClearTimer(ScanInterval_Timer);
	}
}
void ATurret::TakeAim(FVector Aimpoint)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	FRotator StartRotation = BarrelPitchPoint->GetForwardVector().Rotation();
	FRotator RotationToHit = Aimpoint.Rotation();
	FRotator PreNormalDelta = RotationToHit - StartRotation;
	FRotator DeltaRotation = PreNormalDelta.GetNormalized(); // Normalizing makes it so it will freely cross the 0 degree local rotation

	float ClampedYaw = FMath::Clamp(DeltaRotation.Yaw, -TurretYawSpeed * DeltaTime, TurretYawSpeed * DeltaTime);
	float ClampedPitch = FMath::Clamp(DeltaRotation.Pitch, -BarrelPitchSpeed * DeltaTime, BarrelPitchSpeed * DeltaTime);

	float NewYaw = StartRotation.Yaw + ClampedYaw;
	float NewPitch = StartRotation.Pitch + ClampedPitch;

	Turret->SetWorldRotation(FRotator(0.f, NewYaw, 0.f));
	BarrelPitchPoint->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));

	if ((RotationToHit - StartRotation).IsNearlyZero(0.5f))
	{
		OpenFire();
	}
}
void ATurret::OpenFire()
{
	if (!GetWorldTimerManager().IsTimerActive(FireRate_Timer))
	{
		BP_PlayFireFX();
		GetWorld()->SpawnActor<AActor>(Ammunition, Barrel->GetSocketTransform("Muzzle"));
		GetWorldTimerManager().SetTimer(FireRate_Timer, FireRate, false);
	}
}
FVector ATurret::FindFiringSolution()
{
	// Find where the player will be based on it's velocity in the time a shell takes to get to it, this will be used as an aim point if the SuggestVelocity fails
	PredictedPlayerPosition =
		ClosestPawn->GetActorLocation() +
		ClosestPawn->GetVelocity() *
		(ClosestPawn->GetVelocity().Normalize() *
		((ClosestPawn->GetActorLocation() - GetActorLocation()).Size() / 20000.0f));

	// TODO: figure out a good way of getting this from the projectile assigned in BP
	double ProjectileInitialSpeed = 20000.f;

	AvrPlayer* vrPawn = Cast<AvrPlayer>(ClosestPawn);
	if (vrPawn)
	{
		FVector PlayerHead = vrPawn->GetHeadsetCam()->GetComponentLocation();
		FVector SuggestedVelocity = FVector::ZeroVector;
		if (UGameplayStatics::SuggestProjectileVelocity(this, SuggestedVelocity, Barrel->GetSocketLocation("Muzzle"), PlayerHead, ProjectileInitialSpeed))
		{
			// For some reason this function will often return true with an absurd value that causes it to point straight up, do a simple check for this error and negate using it
			if (SuggestedVelocity.Z > 9900.f)
			{
				return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
			}
			else
			{
				return SuggestedVelocity;
			}
		}
		else
		{
			return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
		}
	}

	// Use engine parabola calculator
	FVector SuggestedVelocity = FVector::ZeroVector;
	if (UGameplayStatics::SuggestProjectileVelocity(this, SuggestedVelocity, Barrel->GetSocketLocation("Muzzle"), ClosestPawn->GetActorLocation(), ProjectileInitialSpeed))
	{
		// For some reason this function will often return true with an absurd value that causes it to point straight up, do a simple check for this error and negate using it
		if (SuggestedVelocity.Z > 9900.f)
		{
			return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
		}
		else
		{
			return SuggestedVelocity;
		}
	}
	else
	{
		return PredictedPlayerPosition - Barrel->GetSocketLocation("Muzzle");
	}
}
