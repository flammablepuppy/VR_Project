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

void ARaceGameMode::HandlePlayerDeath(AActor* DyingActor)
{
	if (LoadedCourse.Num() > 0)
	{
		CourseFinished();
	}

	if (CurrentCheckpoint)
	{
		AvrPlayer* vrP = Cast<AvrPlayer>(DyingActor);
		if (vrP) { vrPlayers.AddUnique(vrP); }
		FTimerHandle Respawn_Timer;
		if (!GetWorldTimerManager().IsTimerActive(Respawn_Timer)) { GetWorldTimerManager().SetTimer(Respawn_Timer, this, &ARaceGameMode::RespawnPlayers, 1.f); }
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
		Player->GetHealthStats()->Respawn();

		// If they require a specific item at this checkpoint, spawn it for them
		EquipRequiredItem(Player, Cast<ACheckpoint>(CurrentCheckpoint)->GetRequiredItem());

		OnRespawn.Broadcast(Player);
	}
}

void ARaceGameMode::EquipRequiredItem(AvrPlayer* PlayerToEquip, TSubclassOf<AvrPickup> ItemToEquip)
{
	if (!PlayerToEquip || !ItemToEquip) { return; }

	// Check what items player is holding in hands/holsters
	TArray<AvrPickup*> Inventory; Inventory.Reset();
	PlayerToEquip->GetUtilityBelt()->GetHolsteredItems(Inventory);
	Inventory.AddUnique(PlayerToEquip->GetLeftHeldObject());
	Inventory.AddUnique(PlayerToEquip->GetRightHeldObject());

	// Check if they lack a required item
	int32 HasItem = 0;
	for (AvrPickup* Item : Inventory)
	{
		if (Item)
		{
			if (Item->IsA(ItemToEquip))
			{
				HasItem += 1;
			}
		}
	}

	// If the player doesn't have the item, spawn it and attach it to an empty holster if available
	if (HasItem == 0)
	{
		AvrPickup* SpawnedItem =
		GetWorld()->SpawnActor<AvrPickup>(ItemToEquip, PlayerToEquip->GetUtilityBelt()->GetComponentLocation(), PlayerToEquip->GetUtilityBelt()->GetComponentRotation());

		AvrHolster* VacantHolster = PlayerToEquip->GetUtilityBelt()->GetVacantHolster(SpawnedItem, true);
		// Attach to a vacant, compatible holster if available
		if (VacantHolster)
		{
			SpawnedItem->OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
			SpawnedItem->Drop();
		}
	}
	// If they already have the required item
	else
	{
		// Do nothing
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

