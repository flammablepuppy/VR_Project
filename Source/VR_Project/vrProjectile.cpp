// Copyright Aviator Games 2019

#include "vrProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "vrPlayer.h"
#include "vrPickup.h"
#include "HealthStats.h"

AvrProjectile::AvrProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("Projectile Mesh");
	ProjectileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FCollisionResponseContainer NewResponses;
	NewResponses.Pawn = ECR_Block;
	NewResponses.WorldStatic = ECR_Block;
	NewResponses.WorldDynamic = ECR_Block;
	NewResponses.Visibility = ECR_Ignore;
	ProjectileMesh->SetCollisionResponseToChannels(NewResponses);
	RootComponent = ProjectileMesh;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");
	ProjectileMovement->InitialSpeed = 20000.f;
	ProjectileMovement->MaxSpeed = 40000.f;
	ProjectileMovement->MaxSimulationTimeStep = 0.05f;

}
void AvrProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMesh->OnComponentHit.AddDynamic(this, &AvrProjectile::ResolveHit);
	
}
//void AvrProjectile::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AvrProjectile::ResolveHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	FDamageEvent Damage;

	AvrPickup* FiringWeapon = Cast<AvrPickup>(GetOwner());
	if (FiringWeapon)
	{
		AvrPlayer* FiringPlayer = Cast<AvrPlayer>(FiringWeapon->GetOwningMC()->GetOwner());
		Hit.Actor->TakeDamage(ProjectileDamage, Damage, FiringPlayer->GetController(), this);
	}
	else
	{
		Hit.Actor->TakeDamage(ProjectileDamage, Damage, nullptr, this);
	}

	Destroy();
}

float AvrProjectile::GetProjectileSpeed()
{
	return ProjectileMovement->InitialSpeed;
}
