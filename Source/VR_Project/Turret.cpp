// Copyright Aviator Games 2019

#include "Turret.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "HealthStats.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	
}
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTargetDetectedInLOS)
	{
		AimAzimuth(ClosestPawn->GetActorLocation());
		AimPitch(ClosestPawn->GetActorLocation());
		OpenFire();
	}
}

// Turret Aiming Functions
void ATurret::ScanTripWire(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Scan TripWire fires"))
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
			UE_LOG(LogTemp, Warning, TEXT("Pawn detected by scan"))
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

			// This trace is bad
			FVector TraceExtention = Barrel->GetSocketLocation("RadarScan") - ClosestPawn->GetActorLocation();
			TraceExtention.GetSafeNormal();
			TraceExtention *= 500.f;

			GetWorld()->LineTraceSingleByChannel(HitObject, Barrel->GetSocketLocation("RadarScan"), ClosestPawn->GetActorLocation() - TraceExtention, ECC_WorldStatic);

			APawn* HitPawn = Cast<APawn>(HitObject.Actor);
			if (HitPawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("Detected pawn in LOS"))
				bTargetDetectedInLOS = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No pawn in LOS"))
				bTargetDetectedInLOS = false;
			}

			if (!GetWorldTimerManager().IsTimerActive(ScanInterval_Timer))
			{
				GetWorldTimerManager().SetTimer(ScanInterval_Timer, this, &ATurret::ScanForPawns, ScanSpeed, true);
			}
		}
	}
	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Scanning ceased"))
		bTargetDetectedInLOS = false;
		GetWorldTimerManager().ClearTimer(ScanInterval_Timer); // If a player isn't in the RadarScanZone, shouldn't need to keep scanning... right?
	}
}
void ATurret::AimAzimuth(FVector AimPoint)
{
	FVector AimDirection = AimPoint - Turret->GetComponentLocation();
	AimDirection.Z = 0.f;
	AimDirection.GetSafeNormal();

	FRotator StartRot = Turret->GetForwardVector().Rotation();
	FRotator TargetRotation = AimDirection.Rotation();
	FRotator DeltaRot = TargetRotation - StartRot;

	Turret->SetRelativeRotation(StartRot + DeltaRot);
	
}
void ATurret::AimPitch(FVector AimPoint)
{
	FVector AimDirection = AimPoint - BarrelPitchPoint->GetComponentLocation();

	float StartPitch = BarrelPitchPoint->GetForwardVector().Rotation().Pitch;
	float TargetPitch = AimDirection.Rotation().Pitch;
	float DeltaPitch = TargetPitch - StartPitch;

	BarrelPitchPoint->SetRelativeRotation(FRotator(StartPitch + DeltaPitch, 0.f, 0.f));

}
void ATurret::OpenFire()
{
	if (!GetWorldTimerManager().IsTimerActive(FireRate_Timer))
	{
		GetWorld()->SpawnActor<AActor>(Ammunition, Barrel->GetSocketTransform("Muzzle"));
		GetWorldTimerManager().SetTimer(FireRate_Timer, FireRate, false);
	}
}
