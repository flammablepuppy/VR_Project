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

	// Activate all Waypoint 0's
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		if (ActorIter->GetWaypointNumber() == 0)
		{
			ActorIter->ActivateWaypoint();
		}
	}
}

// Death and Respawning
void ARaceGameMode::HandlePlayerDeath(AActor* DyingActor)
{
	// In this GameMode we don't want players to retain items they had on death, if they've hit a checkpoint the needed items will spawn on their belt
	AvrPlayer* DyingPlayer = Cast<AvrPlayer>(DyingActor);
	if (DyingPlayer)
	{
		if (bShouldYardSale)
		{
			DyingPlayer->GetHealthStats()->YardSale(3.f);
		}
		else
		{
			if (DyingPlayer->GetLeftHeldObject())
			{
				DyingPlayer->GetLeftHeldObject()->Drop();
			}
			if (DyingPlayer->GetRightHeldObject())
			{
				DyingPlayer->GetRightHeldObject()->Drop();
			}
		}
	}

	// If the current checkpoint is a waypoint, it's not a true race course so don't reset all the waypoints
	if (LoadedCourse.Num() > 0 && !CurrentCheckpoint)
	{
		CourseFinished();
	}

	// Respawn player at checkpoint if available
	if (CurrentCheckpoint)
	{
		AvrPlayer* vrP = Cast<AvrPlayer>(DyingActor);
		if (vrP) { vrPlayers.AddUnique(vrP); }
		FTimerHandle Respawn_Timer;
		if (!GetWorldTimerManager().IsTimerActive(Respawn_Timer)) { GetWorldTimerManager().SetTimer(Respawn_Timer, this, &ARaceGameMode::RespawnPlayers, 1.5f); }
	}
	else
	{
		FTimerHandle LoadLevel_Handle;
		FTimerDelegate LoadLevelDelegate;
		FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld());
		LoadLevelDelegate.BindUFunction(this, FName("ModeOpenLevel"), CurrentLevel);

		GetWorldTimerManager().SetTimer(LoadLevel_Handle, LoadLevelDelegate, 2.f, false);
	}

}
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
			if (Cast<AWaypointMarker>(CurrentCheckpoint))
			{
				EquipRequiredItem(Player, Cast<AWaypointMarker>(CurrentCheckpoint)->GetItemsToSpawn());
			}
		}
	}
}
void ARaceGameMode::EquipRequiredItem(AvrPlayer* PlayerToEquip, TArray<TSubclassOf<AvrPickup>> ItemsToEquip)
{
	if (!PlayerToEquip || ItemsToEquip.Num() == 0) { return; }

	TArray<TSubclassOf<AvrPickup>> SpawnItems = ItemsToEquip; // For some reason I have to copy the list, can't figure out how to manipulate the list I took as a parameter

	// Check if player already has item, remove it from the list of items to spawn if they already have it
	TArray<AvrPickup*> HolsteredItems;
	PlayerToEquip->GetUtilityBelt()->GetHolsteredItems(HolsteredItems);
	if (PlayerToEquip->GetLeftHeldObject())
		HolsteredItems.AddUnique(PlayerToEquip->GetLeftHeldObject());
	if (PlayerToEquip->GetRightHeldObject())
		HolsteredItems.AddUnique(PlayerToEquip->GetRightHeldObject());

	if (HolsteredItems.Num() > 0)
	{
		for (AvrPickup* Item : HolsteredItems)
		{
			for (TSubclassOf<AvrPickup> PickupSubclass : ItemsToEquip)
			{
				if (Item->IsA(PickupSubclass))
				{
					SpawnItems.Remove(PickupSubclass);
				}
			}
		}
	}

	// Spawn the item and attach is to player utility belt
	for (TSubclassOf<AvrPickup> ItemToEquip : SpawnItems)
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
}
void ARaceGameMode::ModeOpenLevel(FString LevelToOpen)
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelToOpen));
}

// Waypoints and Races
void ARaceGameMode::HideAllWaypoints()
{
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		ActorIter->DeactivateWaypoint();
	}
}
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
			Message += FString::SanitizeFloat(CheckpointTime);*/
			OnMessageSend.Broadcast("You won.");

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
void ARaceGameMode::LoadCourse(FColor ColorCourseToLoad)
{
	// Set important variables
	CurrentWaypoint = 0;
	LoadedCourse.Reset();
	TimeBetweenWaypoints.Reset();
	CourseStartTime = GetWorld()->GetTimeSeconds();

	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		// Waypoint of the correct color are added to array
		AWaypointMarker* Marker = *ActorIter;
		if (Marker->GetCourseColor() == ColorCourseToLoad)
		{
			LoadedCourse.AddUnique(Marker);
			Marker->OnCollected.AddDynamic(this, &ARaceGameMode::DisplayCurrentWaypoint);
		}
		// Waypoints of other courses are unsubbed and deactivated
		else if (Marker->GetCourseColor() != ColorCourseToLoad)
		{
			Marker->OnCollected.Clear();
			Marker->DeactivateWaypoint();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Course loaded, %d points in course"), LoadedCourse.Num() - 1)
}
void ARaceGameMode::DisplayCurrentWaypoint()
{
	// Course time flags
	float CheckpointTime = GetWorld()->GetTimeSeconds() - CourseStartTime;
	TimeBetweenWaypoints.AddUnique(CheckpointTime);

	CurrentWaypoint++;

	for (AWaypointMarker* Marker : LoadedCourse)
	{
		if (Marker->GetWaypointNumber() == CurrentWaypoint)
		{
			Marker->ActivateWaypoint();
			TargetWaypoint = Marker;
			UE_LOG(LogTemp, Warning, TEXT("%s is active."), *Marker->GetName())

		}

		if (CurrentWaypoint == LoadedCourse.Num())
		{
			CourseFinished();
			TargetWaypoint = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("Course finished called."))
		}
	}
}

// Simple setters
void ARaceGameMode::SetActiveCheckpoint(AActor * CheckpointActor)
{
	CurrentCheckpoint = CheckpointActor;
}
void ARaceGameMode::SetRespawnWithInventory(bool NewState)
{
	bRespawnWithInventory = NewState;
}
void ARaceGameMode::SetShouldYardSale(bool NewState)
{
	bShouldYardSale = NewState;
}
void ARaceGameMode::SetTargetWaypoint(AWaypointMarker* Waypoint)
{
	TargetWaypoint = Waypoint;
}
void ARaceGameMode::SetCurrentWaypoint(int32 WaypointNumber)
{
	CurrentWaypoint = WaypointNumber;
}

