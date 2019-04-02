// Copyright Aviator Games 2019

#include "FloatingMine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPlayer.h"
#include "TimerManager.h"
#include "HealthStats.h"

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

	if (TargetPlayer) { HomeTowardTarget(); }

}
void AFloatingMine::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//		PRIVATE FUNCTIONS
//

void AFloatingMine::OverlapScan(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPlayer* vrPlayer = Cast<AvrPlayer>(OtherActor);
	if (vrPlayer)
	{
		ScanForTargets();
	}
}
void AFloatingMine::ScanForTargets()
{
	TSet<AActor*> ActorsInRadius;
	ScanRadius->GetOverlappingActors(ActorsInRadius);

	for (AActor* Actor : ActorsInRadius)
	{
		AvrPlayer* vrPlayer = Cast<AvrPlayer>(Actor);
		if (vrPlayer && !TargetPlayer)
		{
			TargetPlayer = vrPlayer;
			//UE_LOG(LogTemp, Warning, TEXT("TargetPlayer found: %s"), *TargetPlayer->GetName())

		}
		else if (vrPlayer)
		{
			float CurTargetDis = (TargetPlayer->GetActorLocation() - GetActorLocation()).Size();
			float NewTargetDis = (vrPlayer->GetActorLocation() - GetActorLocation()).Size();
			if (NewTargetDis < CurTargetDis)
			{
				TargetPlayer = vrPlayer;
			}
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
			FDamageEvent Damage;
			HitActor->TakeDamage(MineDamage, Damage, nullptr, this);
			Destroy();

		}
	}
}


