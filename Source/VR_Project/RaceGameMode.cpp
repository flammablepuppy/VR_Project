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
#include "Checkpoint.h"

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

/**
*	Reaction when a player death has been broadcast.
*/
void ARaceGameMode::HandlePlayerDeath(AActor* DyingActor)
{
	// In this GameMode we don't want players to retain items they had on death, if they've hit a checkpoint the needed items will spawn on their belt
	AvrPlayer* DyingPlayer = Cast<AvrPlayer>(DyingActor);
	if (DyingPlayer)
	{
		DyingPlayer->GetHealthStats()->YardSale(3.f);
	}

	// If the current checkpoint is a waypoint, it's not a true race course so don't reset all the waypoints
	AWaypointMarker* WaypointCheckpoint = Cast<AWaypointMarker>(CurrentCheckpoint);
	if (LoadedCourse.Num() > 0 && !WaypointCheckpoint)
	{
		CourseFinished();
	}
	// If using a waypoint checkpoint, make the first waypoint after the checkpoint the active checkpoint
	if (WaypointCheckpoint)
	{
		HideAllWaypoints();
		CurrentWaypoint = WaypointCheckpoint->GetWaypointNumber();
		DisplayCurrentWaypoint();
	}

	if (CurrentCheckpoint)
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

/** Respawn player, called by HandlePlayerDeath when CurrentCheckpoint is not nullptr */
void ARaceGameMode::RespawnPlayers()
{
	for (AvrPlayer* Player : vrPlayers)
	{
		// Reposition, respawn
		Player->SetActorLocation(CurrentCheckpoint->GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Player->GetCharacterMovement()->UpdateComponentVelocity();
		Player->GetHealthStats()->Respawn();

		// If they require a specific item at this checkpoint, spawn it for them
		if (bRespawnWithInventory)
		{
			// Probably best to just keep track of the dropped items, then teleport them back to the player
		}
		else // If the last checkpoint they hit spawns a specific item that is requried, Waypoints cannot spawn required items
		{
			if (Cast<ACheckpoint>(CurrentCheckpoint))
			{
				EquipRequiredItem(Player, Cast<ACheckpoint>(CurrentCheckpoint)->GetRequiredItem());
			}
		}
	}
}

void ARaceGameMode::EquipRequiredItem(AvrPlayer* PlayerToEquip, TArray<TSubclassOf<AvrPickup>> ItemsToEquip)
{
	if (!PlayerToEquip || ItemsToEquip.Num() == 0) { return; }

	for (TSubclassOf<AvrPickup> ItemToEquip : ItemsToEquip)
	{
		AvrPickup* SpawnedItem =
			GetWorld()->SpawnActor<AvrPickup>(ItemToEquip, PlayerToEquip->GetUtilityBelt()->GetComponentLocation(), PlayerToEquip->GetUtilityBelt()->GetComponentRotation());

		AvrHolster* VacantHolster = PlayerToEquip->GetUtilityBelt()->GetVacantHolster(SpawnedItem, true);
		// Attach to a vacant, compatible holster if available -- All holsters should be vacant due to YardSale
		if (VacantHolster)
		{
			SpawnedItem->OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
			SpawnedItem->Drop();
		}
	}
}

void ARaceGameMode::HideAllWaypoints()
{
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		ActorIter->DeactivateWaypoint();
	}
}

/** End race and calculate times/performance */
void ARaceGameMode::CourseFinished()
{
	if (LoadedCourse.Num() == TimeBetweenWaypoints.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Course completed!"))

		for (int32 i = 0; i < TimeBetweenWaypoints.Num(); i++)
		{
			int32 iCount = i;
			float CheckpointTime = TimeBetweenWaypoints[i];
			UE_LOG(LogTemp, Warning, TEXT("Time to checkpoint %d: %f"), iCount, CheckpointTime)

			// TODO: Figure out how to do this properly
			/*FString Message = "Time to checkpoint ";
			Message += FString::FromInt(iCount);
			Message += ": ";
			Message += FString::SanitizeFloat(CheckpointTime);
			OnMessageSend.Broadcast(Message);*/

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Course not completed."))
	}

	// Reset all initial waypoints so courses can be run again
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
			Marker->ActivateWaypoint();
		}
	}
	
	if (CurrentWaypoint == LoadedCourse.Num())
	{
		CourseFinished();
	}
}

void ARaceGameMode::SetActiveCheckpoint(AActor * CheckpointActor)
{
	//CurrentCheckpoint->Destroy();
	CurrentCheckpoint = CheckpointActor;
}

void ARaceGameMode::SetRespawnWithInventory(bool NewState)
{
	bRespawnWithInventory = NewState;
}

