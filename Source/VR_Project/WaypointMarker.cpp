// Copyright Aviator Games 2019


#include "WaypointMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "RaceGameMode.h"
#include "Components/AudioComponent.h"

AWaypointMarker::AWaypointMarker()
{
	WaypointRoot = CreateDefaultSubobject<USceneComponent>("Waypoint Root");
	RootComponent = WaypointRoot;

	WaypointMesh = CreateDefaultSubobject<UStaticMeshComponent>("Waypoint Mesh");
	WaypointMesh->SetupAttachment(RootComponent);
	WaypointMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

	WaypointCollectionSphere = CreateDefaultSubobject<USphereComponent>("Waypoint Collection Sphere");
	WaypointCollectionSphere->SetupAttachment(RootComponent);

	CollectingTone = CreateDefaultSubobject<UAudioComponent>("Collecting Tone");
	CollectingTone->SetupAttachment(RootComponent);
}
void AWaypointMarker::BeginPlay()
{
	Super::BeginPlay();

	WaypointCollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaypointMarker::WaypointReached);
	WaypointCollectionSphere->OnComponentEndOverlap.AddDynamic(this, &AWaypointMarker::WaypointLeft);
	DeactivateWaypoint();

	if (bActivatePointerOnBeginPlay)
	{
		ARaceGameMode* RaceMode = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
		if (RaceMode)
		{
			RaceMode->SetTargetWaypoint(this);
		}
	}

	CollectingTone->SetVolumeMultiplier(0.f);
}
void AWaypointMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCollecting)
	{
		CollectingTone->SetVolumeMultiplier(1.f);
		CollectingPitchChange += DeltaTime * (3/CheckpointCollectionTime);
		CollectingTone->SetPitchMultiplier(CollectingPitchChange);
	}

	if (!bIsCollecting)
	{
		CollectingTone->SetVolumeMultiplier(0.f);
		CollectingPitchChange = 1.f;
		CollectingTone->SetPitchMultiplier(CollectingPitchChange);
	}
}


void AWaypointMarker::WaypointReached(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPlayer* OverlappingPlayer = Cast<AvrPlayer>(OtherActor);
	if (OverlappingPlayer && bWaypointIsActive)
	{
		if(!bTimedCollection)
		{
			AdvanceCourse(OtherActor);
		}
		else
		{
			bIsCollecting = true;
			FTimerDelegate Collection_Delegate;
			Collection_Delegate.BindUFunction(this, FName("AdvanceCourse"), OtherActor);
			GetWorldTimerManager().SetTimer(Collection_Timer, Collection_Delegate, CheckpointCollectionTime, false);
		}
	}
}
void AWaypointMarker::WaypointLeft(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetWorldTimerManager().IsTimerActive(Collection_Timer))
	{
		bIsCollecting = false;
		GetWorldTimerManager().ClearTimer(Collection_Timer);
	}
}
void AWaypointMarker::AdvanceCourse(AActor* CollectingActor)
{
	bIsCollecting = false;
	
	ARaceGameMode* RaceMode = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
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
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CollectionSound, CollectingActor->GetActorLocation());

	if (bFunctionsAsCheckpoint)
		RaceMode->SetActiveCheckpoint(this);
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
