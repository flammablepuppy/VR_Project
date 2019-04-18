// Copyright Aviator Games 2019


#include "RaceGameMode.h"
#include "Engine\World.h"
#include "EngineUtils.h"
#include "WaypointMarker.h"
#include "vrPlayer.h"
#include "HealthStats.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "vrBelt.h"
#include "HandThruster.h"
#include "vrHolster.h"

ARaceGameMode::ARaceGameMode()
{

}
void ARaceGameMode::BeginPlay()
{
	// Sub to all players OnDeath
	for (TActorIterator<AvrPlayer> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		ActorIter->GetHealthStats()->OnDeath.AddDynamic(this, &ARaceGameMode::HandlePlayerDeath);
	}
}

void ARaceGameMode::HandlePlayerDeath(AActor* DyingActor)
{
	if (LoadedCourse.Num() > 0)
	{
		CourseFinished();
	}

	if (ActiveCheckpoint != FVector::ZeroVector)
	{
		AvrPlayer* vrP = Cast<AvrPlayer>(DyingActor);
		if (vrP) { vrPlayers.AddUnique(vrP); }
		FTimerHandle Respawn_Timer;
		if (!GetWorldTimerManager().IsTimerActive(Respawn_Timer)) { GetWorldTimerManager().SetTimer(Respawn_Timer, this, &ARaceGameMode::RespawnPlayers, 2.f); }
	}
	else
	{
		FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld());
		UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevel));
	}

}

/** Respawn player and make sure they have a hand thruster */
void ARaceGameMode::RespawnPlayers()
{
	for (AvrPlayer* Player : vrPlayers)
	{
		// Respawn, reposition, zero velocity
		Player->SetActorLocation(ActiveCheckpoint);
		Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Player->GetCharacterMovement()->UpdateComponentVelocity();
		Player->GetHealthStats()->Respawn();

		// Check what items player is holding in hands/holsters
		TArray<AvrPickup*> Inventory; Inventory.Reset();
		Player->GetUtilityBelt()->GetHolsteredItems(Inventory);
		Inventory.AddUnique(Player->GetLeftHeldObject());
		Inventory.AddUnique(Player->GetRightHeldObject());

		// Check if they lack a required item
		if (!AddToInventoryOnRespawn) { UE_LOG(LogTemp, Warning, TEXT("No required item set")) OnRespawn.Broadcast(Player); return; }
		else { UE_LOG(LogTemp, Warning, TEXT("Need a %s"), *AddToInventoryOnRespawn->GetDefaultObject()->GetName()) }

		int32 HasItem = 0;
		for (AvrPickup* Item : Inventory)
		{
			if (Item)
			{
				UE_LOG(LogTemp, Warning, TEXT("Item found: %s"), *Item->GetName())

				if (Item->IsA(AddToInventoryOnRespawn))
				{
					HasItem += 1;
					UE_LOG(LogTemp, Warning, TEXT("Required Item Found"))
				}
			}
		}

		// If you already have the required item do nothing, otherwise spawn it and attach it to the players belt
		if (HasItem == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Missing required item, attempting to spawn"))

			AvrPickup* SpawnedItem =
				GetWorld()->SpawnActor<AvrPickup>(AddToInventoryOnRespawn, Player->GetUtilityBelt()->GetComponentLocation(), Player->GetUtilityBelt()->GetComponentRotation());

			AvrHolster* VacantHolster = Player->GetUtilityBelt()->GetVacantHolster(SpawnedItem, true);
			// Attach to a vacant, compatible holster if available
			if (VacantHolster)
			{
				UE_LOG(LogTemp, Warning, TEXT("Holstering cast succeeds"))

				SpawnedItem->OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
				SpawnedItem->Drop();
			}
		}
		else
		{
			// Do nothing
			UE_LOG(LogTemp, Warning, TEXT("No need to spawn"))
		}

		OnRespawn.Broadcast(Player);
	}
}

/** End race and calculate times/performance */
void ARaceGameMode::CourseFinished()
{
	if (LoadedCourse.Num() == TimeBetweenWaypoints.Num())
	{
		for (int32 i = 0; i < TimeBetweenWaypoints.Num(); i++)
		{
			int32 iCount = i;
			float CheckpointTime = TimeBetweenWaypoints[i];
			UE_LOG(LogTemp, Warning, TEXT("Time to checkpoint %d: %f"), iCount, CheckpointTime)
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Course Finished!"))

	/** Reset all initial waypoints so courses can be run again */
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		if (ActorIter->GetWaypointNumber() == 0)
		{
			ActorIter->ActivateWaypoint();
		}
		else
		{
			ActorIter->DeactivateWaypoint();
		}

		ActorIter->OnCollected.Clear();
		CurrentWaypoint = 0;
	}
}

/** The '0' waypoint for a course is what loads the course */
void ARaceGameMode::LoadCourse(FColor ColorCourseToLoad)
{
	LoadedCourse.Reset();
	TimeBetweenWaypoints.Reset();
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		AWaypointMarker* Marker = *ActorIter; 
		if (Marker->GetCourseColor() == ColorCourseToLoad)
		{
			CourseStartTime = GetWorld()->GetTimeSeconds();
			LoadedCourse.AddUnique(Marker);
			Marker->OnCollected.AddDynamic(this, &ARaceGameMode::DisplayCurrentWaypoint);
		}
		else if (Marker->GetCourseColor() != ColorCourseToLoad)
		{
			Marker->OnCollected.Clear();
			Marker->DeactivateWaypoint();
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Course loaded, %d points in course"), LoadedCourse.Num() - 1)
}

/** The first call to this function will be on the 0 waypoint for any given course */
void ARaceGameMode::DisplayCurrentWaypoint()
{
	float CheckpointTime = GetWorld()->GetTimeSeconds() - CourseStartTime;
	TimeBetweenWaypoints.AddUnique(CheckpointTime);

	CurrentWaypoint += 1;

	for (AWaypointMarker* Marker : LoadedCourse)
	{
		if (Marker->GetWaypointNumber() == CurrentWaypoint)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Current Waypoint: %d"), CurrentWaypoint)
			Marker->ActivateWaypoint();
		}
	}
	
	if (CurrentWaypoint == LoadedCourse.Num())
	{
		CourseFinished();
	}
}

void ARaceGameMode::SetActiveCheckpoint(FVector CheckpointLocation)
{
	ActiveCheckpoint = CheckpointLocation;
}

