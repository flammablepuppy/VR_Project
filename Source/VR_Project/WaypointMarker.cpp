// Copyright Aviator Games 2019


#include "WaypointMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPlayer.h"
#include "HealthStats.h"
#include "Kismet/GameplayStatics.h"
#include "RaceGameMode.h"

AWaypointMarker::AWaypointMarker()
{
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
	DeactivateWaypoint();
}

/**
*	
*/

void AWaypointMarker::WaypointReached(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ARaceGameMode* RaceMode = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
	AvrPlayer* OverlappingPlayer = Cast<AvrPlayer>(OtherActor);
	if (OverlappingPlayer && bWaypointIsActive)
	{
		if (WaypointNumber == 0)
			RaceMode->LoadCourse(CourseColor);

		if (bFinalWaypoint)
		{
			RaceMode->SetTargetWaypoint(nullptr);
			RaceMode->SetCurrentWaypoint(-1);
		}
		else
			OnCollected.Broadcast();

		DeactivateWaypoint();
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), CollectionSound, OverlappingPlayer->GetActorLocation());

		if (bFunctionsAsCheckpoint)
			RaceMode->SetActiveCheckpoint(this);
	}
}
void AWaypointMarker::ActivateWaypoint()
{
	bWaypointIsActive = true;
	WaypointMesh->SetVisibility(true);
}
void AWaypointMarker::DeactivateWaypoint()
{
	bWaypointIsActive = false;
	WaypointMesh->SetVisibility(false);
}
