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
	
	ArrowMesh->SetVisibility(false);

	ARaceGameMode* TheMode = Cast<ARaceGameMode>(UGameplayStatics::GetGameMode(this));
	if (TheMode)
	{
		FTimerHandle Pointer_Ticker;
		GetWorldTimerManager().SetTimer(Pointer_Ticker, this, &AWaypointFinder::Point, 1.f / 45.f, true, 0.f);
	}

}

void AWaypointFinder::Point()
{
	ARaceGameMode* TheMode = Cast<ARaceGameMode>(UGameplayStatics::GetGameMode(this));
	if (TheMode->GetTargetWaypoint())
	{
		AWaypointMarker* TargetWP = Cast<AWaypointMarker>(TheMode->GetTargetWaypoint());
		if (TargetWP)
		{
			// Display arrow
			if (!ArrowMesh->IsVisible()) ArrowMesh->SetVisibility(true);

			// Match arrow material to waypoint 
			if (ArrowMaterial == nullptr)
				ArrowMaterial = ArrowMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(1, TargetWP->GetWaypointMesh()->GetMaterial(1));

			// Point toward waypoint
			FVector TargetLocation = TheMode->GetTargetWaypoint()->GetActorLocation();
			FVector FromLocation = GetActorLocation();
			FVector Direction = (TargetLocation - FromLocation).GetSafeNormal();
			FRotator Rotation = Direction.Rotation();
			ArrowMesh->SetWorldRotation(Rotation);
		}
	}
	else
	{
		ArrowMesh->SetVisibility(false);
		ArrowMaterial = nullptr;
	}

}

