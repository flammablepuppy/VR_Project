// Copyright Aviator Games 2019


#include "WaypointFinder.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RaceGameMode.h"
#include "vrPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "WaypointMarker.h"

AWaypointFinder::AWaypointFinder()
{
	PrimaryActorTick.bCanEverTick = true;

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>("Arrow Mesh");
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowMesh->SetSimulatePhysics(false);
	ArrowMesh->SetCastShadow(false);
	RootComponent = ArrowMesh;

}
void AWaypointFinder::BeginPlay()
{
	Super::BeginPlay();
	
	ARaceGameMode* GM = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		
		FTimerHandle Pointer_Ticker;
		GetWorldTimerManager().SetTimer(Pointer_Ticker, this, &AWaypointFinder::Point, 1.f / /* TODO: Make this variable based on the headset in use, Quest 1 display runs at 72hz */72.f, true, 0.f);
		GM->OnCourseLoaded.AddUniqueDynamic(this, &AWaypointFinder::HandleCourseLoaded);
		GM->OnCourseComplete.AddUniqueDynamic(this, &AWaypointFinder::HandleCourseCompleted);
	}

}

void AWaypointFinder::Point()
{
	ARaceGameMode* GM = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
	if (GM->GetTargetWaypoint())
	{
		const AWaypointMarker* Waypoint = GM->GetTargetWaypoint();
		if (Waypoint->GetWaypointIsActive())
		{
			// Point toward waypoint
			const FVector TargetLocation = GM->GetTargetWaypoint()->GetActorLocation();
			const FVector FromLocation = GetActorLocation();
			const FVector Direction = (TargetLocation - FromLocation).GetSafeNormal();
			ArrowMesh->SetWorldRotation(Direction.Rotation());
		}
	}
	else
	{
		ArrowMesh->SetVisibility(false);
		ArrowMaterial = nullptr;
	}

}
void AWaypointFinder::HandleCourseLoaded(AWaypointMarker* CourseWaypoint)
{
	//UE_LOG(LogTemp, Warning, TEXT("Arrow turns on"))
	ArrowMesh->SetVisibility(true, true);
	ArrowMaterial = ArrowMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(1, CourseWaypoint->GetWaypointMesh()->GetMaterial(1));
}
void AWaypointFinder::HandleCourseCompleted()
{
	//UE_LOG(LogTemp, Warning, TEXT("Arrow turns off"))
	ArrowMesh->SetVisibility(false);
}

