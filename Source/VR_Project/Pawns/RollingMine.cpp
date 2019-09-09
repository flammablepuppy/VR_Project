// Copyright Aviator Games 2019


#include "RollingMine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "HealthStats.h"
#include "vrPlayer.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Sound/SoundCue.h"

/**
*
*/

ARollingMine::ARollingMine()
{
	PrimaryActorTick.bCanEverTick = true;

	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mine Mesh");
	MineMesh->SetSimulatePhysics(true);
	MineMesh->SetGenerateOverlapEvents(true);
	MineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = MineMesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>("Trigger Sphere");
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetSphereRadius(5000.f);
	TriggerSphere->SetupAttachment(RootComponent);

	ExplosionSphere = CreateDefaultSubobject<USphereComponent>("Explosion Sphere");
	ExplosionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExplosionSphere->SetSphereRadius(175.f);
	ExplosionSphere->SetupAttachment(RootComponent);

	MineHealth = CreateDefaultSubobject<UHealthStats>("Mine Health");

}
void ARollingMine::BeginPlay()
{
	Super::BeginPlay();

	MineHealth->OnDeath.AddDynamic(this, &ARollingMine::Explode);
	MineMesh->OnComponentHit.AddDynamic(this, &ARollingMine::SpikeStab);
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
		if (vrPlayer && !vrPlayer->GetHealthStats()->GetIsDead())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Target location: %s"), *Player->GetActorLocation().ToString())
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + (FVector::UpVector * 75.f), Player->GetActorLocation(), ECC_Camera, Params);

			if (Cast<AvrPlayer>(Hit.Actor))
			{
				VisiblePlayers.AddUnique(vrPlayer);
				//UE_LOG(LogTemp, Warning, TEXT("Player added to VisiblePlayers array"), *Hit.Actor->GetName())
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("LOS Trace Hit: %s"), *Hit.GetComponent()->GetName())
			}
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
FVector ARollingMine::FindTargetDirection()
{
	FVector BallLocation = GetActorLocation();
	FVector TargetLocation = TargetPlayer->GetActorLocation();
	
	FVector BallFutureLocation = BallLocation + GetVelocity();
	FVector TargetFutureLocation = TargetLocation + TargetPlayer->GetVelocity();

	FVector TrackingDirection = (TargetLocation - BallLocation).GetSafeNormal() + (TargetFutureLocation - BallFutureLocation).GetSafeNormal();
	TrackingDirection.Z = 0.f;

	return TrackingDirection;
}
float ARollingMine::FindMoveForce()
{
	float PercentMaxSpeed = GetVelocity().Size() / MoveMaxSpeed;
	float ForceAvailable = MaxMoveForce;

	// Interp ForceAvailable to zero as velocity approaches MoveMaxSpeed
	if (PercentMaxSpeed > BrakeOnset)
		ForceAvailable -= (MaxMoveForce * PercentMaxSpeed);

	// Don't move when SpikeCooldown is active or when moving through the air faster than max speed
	// Move slower when airborne
	if (GetWorldTimerManager().IsTimerActive(SpikeCooldown_Timer) || CheckIsAirbourne() && PercentMaxSpeed > BrakeOnset / 2.f)
	{
		return 0.f;
	}
	else if (CheckIsAirbourne())
	{
		return ForceAvailable / 5;
	}
	else
	{
		return ForceAvailable;
	}
}
void ARollingMine::MoveToTarget()
{
	FVector Direction = FindTargetDirection();
	float MoveForce = FindMoveForce();
	float PercentMaxSpeed = GetVelocity().Size() / MoveMaxSpeed;
	float DistanceToTarget = (TargetPlayer->GetActorLocation() - GetActorLocation()).Size();

	// Determine how close to max speed
	float PerMaxSpeed = GetVelocity().Size() / MoveMaxSpeed;

	//Determine angle between velocity and target
	FRotator DirRot = Direction.Rotation();
	FVector Line = TargetPlayer->GetActorLocation() - GetActorLocation();
	FRotator LineRot = Line.Rotation();
	FRotator PreNormal = LineRot - DirRot;
	FRotator CheckRot = PreNormal.GetNormalized();
	float AngleCheck = FMath::Abs(CheckRot.Yaw);
	

	// Pump brakes if target is outside allowable angle and near max speed, except when within 450cm
	if (AngleCheck > AllowableAngle && PercentMaxSpeed > BrakeOnset && DistanceToTarget > 450.f)
	{
		MineMesh->AddForce(-GetVelocity().GetSafeNormal() * BrakingForce, NAME_None, true);
	}
	else
	{
		MineMesh->AddForce(Direction * MoveForce, NAME_None, true);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Ball Velocity: %f | Distance: %f | MoveForce: %f | Off Angle: %f"), GetVelocity().Size(), DistanceToTarget, MoveForce, AngleCheck)

}
bool ARollingMine::CheckIsAirbourne()
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + (FVector::DownVector * 75.f), ECC_Camera, Params))
	{
		return false;
	}
	else
	{
		return true;
	}
}
void ARollingMine::Explode(AActor* DyingActor)
{
	if (bHasExploded) return;

	TSet<AActor*> AffectedActors;
	ExplosionSphere->GetOverlappingActors(AffectedActors);

	for (AActor* HitActor : AffectedActors)
	{
		FDamageEvent DamEve;
		HitActor->TakeDamage(ExplosionDamage, DamEve, this->GetController(), this);	
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorTransform());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	SetLifeSpan(0.001f);	
}
void ARollingMine::SpikeStab(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/** Damage impacted player and apply impulse */
	AvrPlayer* Player = Cast<AvrPlayer>(OtherComp->GetOwner());
	if (Player && !GetWorldTimerManager().IsTimerActive(SpikeCooldown_Timer))
	{

		FDamageEvent DamEve;
		OtherActor->TakeDamage(SpikeDamage, DamEve, this->GetController(), this);

		Player->GetHealthStats()->BleedOn(BleedDamage, 3.f, 2.f);

		FVector ImpulseDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal() + FVector(0.f, 0.f, 1.f);
		Player->GetMovementComponent()->Velocity += ImpulseDirection.GetSafeNormal() * ImpulsePower;
		Player->GetMovementComponent()->UpdateComponentVelocity();
		MineMesh->AddImpulse(-ImpulseDirection*3.f);

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), StabSound, GetActorLocation());
		GetWorldTimerManager().SetTimer(SpikeCooldown_Timer, SpikeCooldownDuration, false);
		//UE_LOG(LogTemp, Log, TEXT("Spike impact!"))

	}
}