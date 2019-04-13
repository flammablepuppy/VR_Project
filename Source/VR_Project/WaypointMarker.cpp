// Copyright Aviator Games 2019


#include "WaypointMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPlayer.h"
#include "HealthStats.h"
#include "Kismet/GameplayStatics.h"

AWaypointMarker::AWaypointMarker()
{
	PrimaryActorTick.bCanEverTick = true;

	WaypointRoot = CreateDefaultSubobject<USceneComponent>("Waypoint Root");
	RootComponent = WaypointRoot;

	WaypointMesh = CreateDefaultSubobject<UStaticMeshComponent>("Waypoint Mesh");
	WaypointMesh->SetupAttachment(RootComponent);
	WaypointMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	WaypointCollectionSphere = CreateDefaultSubobject<USphereComponent>("Waypoint Collection Sphere");
	WaypointCollectionSphere->SetupAttachment(RootComponent);

}

void AWaypointMarker::BeginPlay()
{
	Super::BeginPlay();

	WaypointCollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaypointMarker::WaypointReached);
	if (bAutoActivates) { ActivateWaypoint(); }
	
}

void AWaypointMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/**
*	
*/

void AWaypointMarker::WaypointReached(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPlayer* OverlappingPlayer = Cast<AvrPlayer>(OtherActor);
	if (OverlappingPlayer && bWaypointActive)
	{
		bWaypointActive = false;

		OverlappingPlayer->GetHealthStats()->AdjustCurrency();
		if (CollectionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectionSound, OverlappingPlayer->GetActorLocation());
		}
		OnCollected.Broadcast();
		SetLifeSpan(EffectsDuration);
	}
}

void AWaypointMarker::ActivateWaypoint()
{
	bWaypointActive = true;
	WaypointMesh->SetVisibility(true);
}

void AWaypointMarker::DeactivateWaypoint()
{
	bWaypointActive = false;
	WaypointMesh->SetVisibility(false);
}

