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

void AWaypointMarker::ScanForPlayer()
{
	TArray<AActor*> Actors;
	WaypointCollectionSphere->GetOverlappingActors(Actors);

	for (AActor* Actor : Actors)
	{
		AvrPlayer* IsPlayer = Cast<AvrPlayer>(Actor);
		if (IsPlayer) return;
	}

	StopCollection();
}
void AWaypointMarker::StopCollection()
{
	if (GetWorldTimerManager().IsTimerActive(Collection_Timer))
	{
		bIsCollecting = false;
		GetWorldTimerManager().ClearTimer(Collection_Timer);
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
			OnBeginCollecting.Broadcast(Collection_Timer);
		}
	}
}
void AWaypointMarker::WaypointLeft(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TArray<AActor*> Actors;
	WaypointCollectionSphere->GetOverlappingActors(Actors);

	for (AActor* Actor : Actors)
	{
		if (OtherActor == Actor) return;
	}

	FTimerHandle DoubleCheck_Timer;
	FTimerDelegate DoubleCheck_Delegate;
	DoubleCheck_Delegate.BindUFunction(this, FName("ScanForPlayer"));
	GetWorldTimerManager().SetTimer(DoubleCheck_Timer, DoubleCheck_Delegate, 0.05f, false);
	
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
