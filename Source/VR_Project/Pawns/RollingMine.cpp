// Copyright Aviator Games 2019


#include "RollingMine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "HealthStats.h"
#include "vrPlayer.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
 
/**
*
*/

ARollingMine::ARollingMine()
{
	PrimaryActorTick.bCanEverTick = true;

	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mine Mesh");
	MineMesh->SetSimulatePhysics(true);
	RootComponent = MineMesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>("Trigger Sphere");
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetSphereRadius(5000.f);
	TriggerSphere->SetupAttachment(RootComponent);

	ExplosionSphere = CreateDefaultSubobject<USphereComponent>("Explosion Sphere");
	ExplosionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExplosionSphere->SetSphereRadius(150.f);
	ExplosionSphere->SetupAttachment(RootComponent);

	MineHealth = CreateDefaultSubobject<UHealthStats>("Mine Health");

}
void ARollingMine::BeginPlay()
{
	Super::BeginPlay();

	MineHealth->OnDeath.AddDynamic(this, &ARollingMine::Explode);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ARollingMine::Search);
	GetWorldTimerManager().SetTimer(LOSTick_Timer, this, &ARollingMine::SeekClosestPlayer, LOSTickInterval, true, 0.f);

}
void ARollingMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// When a target is set, add force in direction of target.
	if (TargetPlayer) { MoveToTarget(); }
	
}
void ARollingMine::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/**
*
*/

void ARollingMine::Search(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/** Start seeking when a vrPlayer enteres the TriggerSphere */
	if (Cast<AvrPlayer>(OtherActor))
	{
		GetWorldTimerManager().SetTimer(LOSTick_Timer, this, &ARollingMine::SeekClosestPlayer, LOSTickInterval, true, 0.f);
	}
}
void ARollingMine::SeekClosestPlayer()
{
	/** Create array from all players in the TriggerSphere */
	TSet<AActor*> PlayersInRange;
	TriggerSphere->GetOverlappingActors(PlayersInRange, AvrPlayer::StaticClass());

	/** If no players are found in the TriggerSphere, stop the search */
	if (PlayersInRange.Num() < 1) { GetWorldTimerManager().ClearTimer(LOSTick_Timer); }
		 
	/** Determine which players are in LOS */
	TArray<AvrPlayer*> VisiblePlayers;
	for (AActor* Player : PlayersInRange)
	{
		AvrPlayer* vrPlayer = Cast<AvrPlayer>(Player);
		if (vrPlayer)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Target location: %s"), *Player->GetActorLocation().ToString())
			FHitResult Hit;
			FVector TraceStart = GetActorLocation();
			FVector TraceEnd = Player->GetActorLocation();
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

			VisiblePlayers.AddUnique(vrPlayer);
			//UE_LOG(LogTemp, Warning, TEXT("Player added to VisiblePlayers array"), *vrPlayer->GetName())
		}
	}

	/** Determine which of the visible players is closest */
	AvrPlayer* ClosestVisiblePlayer = nullptr;
	for (AvrPlayer* Player : VisiblePlayers)
	{
		float DistanceToPlayer = (GetActorLocation() - Player->GetActorLocation()).Size();

		/** If this is the first tested player, immediately set as ClosestVisiblePlayer */
		if (!ClosestVisiblePlayer)
		{
			ClosestVisiblePlayer = Player;
		}
		/** If subsequent tested distances are shorter then overwrite the ClosestVisiblePlayer */
		else if (DistanceToPlayer < (GetActorLocation() - ClosestVisiblePlayer->GetActorLocation()).Size())
		{
			ClosestVisiblePlayer = Player;
		}
	}

	TargetPlayer = ClosestVisiblePlayer;
}
void ARollingMine::MoveToTarget()
{
	FVector PredictedRequirement = TargetPlayer->GetActorLocation() - (GetActorLocation() + GetVelocity());
	FVector StraightShot = TargetPlayer->GetActorLocation() - GetActorLocation();
	FVector Direction = FVector::ZeroVector;

	FVector PredictedCheck = (GetActorLocation() + PredictedRequirement).GetSafeNormal();
	FVector StraightCheck = (GetActorLocation() + StraightShot).GetSafeNormal();
	float Tolerance = (PredictedCheck - StraightCheck).Size();


	//float AngleBetweenVectors = 
	//	FMath::Acos(
	//		FVector::DotProduct(StraightShot.GetSafeNormal(), GetVelocity().GetSafeNormal()) / (StraightShot.Size() * GetVelocity().Size())
	//	);

	////UE_LOG(LogTemp, Warning, TEXT("%f"), FVector::DotProduct(StraightShot.GetSafeNormal(), GetVelocity().GetSafeNormal()))
	float DotPro = FVector::DotProduct(StraightShot.GetSafeNormal(), GetVelocity().GetSafeNormal());

	if (Tolerance > 0.025f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Speed: %f | StraightShot"), GetVelocity().Size())
		Direction = StraightShot;
	}
	else if (DotPro < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Speed: %f | Braking | %f"), GetVelocity().Size(), DotPro)
		Direction = -GetVelocity().GetSafeNormal();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Speed: %f | Predicting"), GetVelocity().Size())
		Direction = TargetPlayer->GetActorLocation() - (GetActorLocation() + GetVelocity());
	}
	Direction.Z = 0.f;
	Direction.GetSafeNormal();

	MineMesh->AddForce(Direction * MoveForce, NAME_None, true);
		
}
void ARollingMine::Explode(AActor* DyingActor)
{
	if (DyingActor == this)
	{
		TSet<AActor*> AffectedActors;
		ExplosionSphere->GetOverlappingActors(AffectedActors, AvrPlayer::StaticClass());

		for (AActor* Player : AffectedActors)
		{
			FDamageEvent DamEve;
			Player->TakeDamage(ExplosionDamage, DamEve, this->GetController(), this);
		}
	}
}
