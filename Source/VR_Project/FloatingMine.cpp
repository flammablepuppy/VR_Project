// Copyright Aviator Games 2019

#include "FloatingMine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPlayer.h"
#include "TimerManager.h"
#include "HealthStats.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

AFloatingMine::AFloatingMine()
{
	PrimaryActorTick.bCanEverTick = true;

	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mine Mesh");
	RootComponent = MineMesh;

	ScanRadius = CreateDefaultSubobject<USphereComponent>("Scan Radius");
	ScanRadius->SetupAttachment(RootComponent);

	BlastRadius = CreateDefaultSubobject<USphereComponent>("Blast Radius");
	BlastRadius->SetupAttachment(RootComponent);

	HealthStats = CreateDefaultSubobject<UHealthStats>("Health Stats");
}
void AFloatingMine::BeginPlay()
{
	Super::BeginPlay();

	ScanRadius->OnComponentBeginOverlap.AddDynamic(this, &AFloatingMine::OverlapScan);
	BlastRadius->OnComponentBeginOverlap.AddDynamic(this, &AFloatingMine::Explode);
	ScanForTargets();
	
}
void AFloatingMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bScanOn) { ScanForTargets(); }

	// Ensure mine has LOS before tracking toward target -- there's almost certainly a better place to put this, probably on a short timer instead of Tick
	if (TargetPlayer && !bHasTarget)
	{
		FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
		Direction.GetSafeNormal();

		//DrawDebugLine(GetWorld(), 
		//	GetActorLocation(), 
		//	GetActorLocation() + Direction * 500.f, 
		//	FColor::Red, 
		//	false, 
		//	0.1f);

		FHitResult Hit;
		float LOSDistance = ScanRadius->GetScaledSphereRadius();
		if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + Direction * LOSDistance, ECC_Pawn))
		{
			//if (Hit.GetActor())	{ UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *Hit.GetActor()->GetName()) }
			AvrPlayer* HitPlayer = Cast<AvrPlayer>(Hit.GetActor());
			if (HitPlayer)
			{
				bHasTarget = true;
			}
		}

	}

	if (bHasTarget && bTrackingOn) { HomeTowardTarget(); }

}
void AFloatingMine::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//		PRIVATE FUNCTIONS
//

void AFloatingMine::OverlapScan(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bScanOn = true;
}

void AFloatingMine::ScanForTargets()
{
	TSet<AActor*> ActorsInRadius;
	ScanRadius->GetOverlappingActors(ActorsInRadius);

	for (AActor* Actor : ActorsInRadius)
	{
		// Check for vrPlayers
		AvrPlayer* vrPlayer = Cast<AvrPlayer>(Actor);

		// Set TargetPlayer to closest vrPlayer
		if (vrPlayer && !TargetPlayer)
		{
			TargetPlayer = vrPlayer;
			//UE_LOG(LogTemp, Warning, TEXT("TargetPlayer found: %s"), *TargetPlayer->GetName())

		}

		// If TargetPlayer is already set, make any player that gets closer than current target the new target
		else if (vrPlayer)
		{
			float CurTargetDis = (TargetPlayer->GetActorLocation() - GetActorLocation()).Size();
			float NewTargetDis = (vrPlayer->GetActorLocation() - GetActorLocation()).Size();
			if (NewTargetDis < CurTargetDis)
			{
				TargetPlayer = vrPlayer;
			}
		}

		// Stop the scan if there are no vrPlayers
		if (!vrPlayer)
		{
			bScanOn = false;
		}

	}
}

void AFloatingMine::HomeTowardTarget()
{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		FVector CurrentLocation = MineMesh->GetComponentLocation();
		FVector TargetLocation = TargetPlayer->GetActorLocation();
		FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

		FVector Acceleration = OldVelocity + Direction * MineAcceleration * DeltaTime;
		if (Acceleration.Size() > MineTopSpeed) { Acceleration *= 0.94f; }

		SetActorLocation(CurrentLocation + Acceleration);
		SetActorRotation(Direction.Rotation());
		OldVelocity = GetActorLocation() - CurrentLocation;

}

void AFloatingMine::Explode(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPlayer* Player = Cast<AvrPlayer>(OtherActor);
	if (Player)
	{
		TSet<AActor*> BlastedActors;
		BlastRadius->GetOverlappingActors(BlastedActors);
		for (AActor* HitActor : BlastedActors)
		{
			// Cast to vrPlayer to prevent from killing other nearby mines
			AvrPlayer* HitPlayer = Cast<AvrPlayer>(HitActor);
			{
				if (HitPlayer)
				{
					FDamageEvent Damage;
					HitActor->TakeDamage(MineDamage, Damage, nullptr, this);
					UGameplayStatics::ApplyDamage(this, HealthStats->GetMaxHealth(), nullptr, nullptr, nullptr);
				}
			}
		}
	}
}

void AFloatingMine::SetTrackingOn(bool NewState)
{
	bTrackingOn = NewState;
	NewState ? MineMesh->SetEnableGravity(false) : MineMesh->SetEnableGravity(true);

}


