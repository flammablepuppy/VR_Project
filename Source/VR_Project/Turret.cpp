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

	UProjectileMovementComponent* ProjComp = Cast<UProjectileMovementComponent>(Ammunition->GetDefaultSubobjectByName(FName("ProjectileMovement")));
	if (ProjComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile speed found!"))
		AmmoSpeed = ProjComp->InitialSpeed;
	}
}
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTargetDetectedInLOS)
	{
		PredictedPlayerPosition = 
			ClosestPawn->GetActorLocation() + 
			ClosestPawn->GetVelocity() * 
			(ClosestPawn->GetVelocity().Normalize() * ((ClosestPawn->GetActorLocation() - GetActorLocation()).Size() / 20000.0f)); // Shoot where the player will be in the time it takes for the bullet to get there

		// This doesn't take into account the rotation of the barrel, only applies gravity compensation for a perfectly level shot... need to rethink this.
		//PredictedPlayerPosition += (FVector(0.f, 0.f, 1.f) * 490.f * ((ClosestPawn->GetActorLocation() - Barrel->GetSocketLocation("Muzzle")).Size() / 20000.0f)); // Compensate for gravity

		// Don't quite understand how to use this yet...
		//FVector SuggestedVelocity;
		//UGameplayStatics::SuggestProjectileVelocity(GetWorld(), SuggestedVelocity, Barrel->GetSocketLocation("Muzzle"), ClosestPawn->GetActorLocation(), 20000.f);

		AimAzimuth(PredictedPlayerPosition);
		AimPitch(PredictedPlayerPosition);

		// TODO: Put firing in an if statment that checks if the forward point vector of the barrel is nearly equal to the point vector required for the projectile to hit the players predicted position
		// ie. only shoot if you have a decent chance of hitting
		OpenFire();
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

			// This trace is bad
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
void ATurret::AimAzimuth(FVector AimPoint)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector AimDirection = AimPoint - Turret->GetComponentLocation();

	float StartYaw = Turret->GetForwardVector().Rotation().Yaw;
	float TargetYaw = AimDirection.Rotation().Yaw;
	float DeltaYaw = TargetYaw - StartYaw;
	float ClampedDeltaYaw = FMath::Clamp(DeltaYaw, -TurretYawSpeed * DeltaTime, TurretYawSpeed * DeltaTime);
	float NewYaw = StartYaw + ClampedDeltaYaw;
	Turret->SetRelativeRotation(FRotator(0.f, NewYaw, 0.f));
	
}
void ATurret::AimPitch(FVector AimPoint)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector AimDirection = AimPoint - BarrelPitchPoint->GetComponentLocation();

	float StartPitch = BarrelPitchPoint->GetForwardVector().Rotation().Pitch;
	float TargetPitch = AimDirection.Rotation().Pitch;
	float DeltaPitch = TargetPitch - StartPitch;
	float ClampedDeltaPitch = FMath::Clamp(DeltaPitch, -BarrelPitchSpeed * DeltaTime, BarrelPitchSpeed * DeltaTime);
	float NewPitch = StartPitch + ClampedDeltaPitch;
	BarrelPitchPoint->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));

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
