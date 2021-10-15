// Copyright Aviator Games 2019


#include "RaceGameMode.h"

#include <Actor.h>

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
#include "Landscape.h"
#include "vrHolster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape/Classes/Landscape.h"

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

// Race Setup Helpers
void ARaceGameMode::PrepPlayerForRace()
{
	for (TActorIterator<AvrPlayer> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		GivePlayerThruster(*ActorItr);

		if (ActorItr->GetUtilityBelt())
		{
			ActorItr->GetUtilityBelt()->DestoryHolsters();
			ActorItr->GetUtilityBelt()->Deactivate();
			ActorItr->GetUtilityBelt()->DestroyComponent();
		}
	}

	// This can only be used if there is a single player start
	for (TActorIterator<APlayerStart> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		SetActiveCheckpoint(*ActorIterator);
	}
	
	bRaceSetup = true;

}
void ARaceGameMode::GivePlayerThruster(AvrPlayer* Player)
{
	if (GivenThruster)
	{
		AActor* NewThruster = GetWorld()->SpawnActor(GivenThruster);
		AHandThruster* Thruster = Cast<AHandThruster>(NewThruster);
		Thruster->SetCanDrop(false);
		Thruster->SetDisableDropOnGrip(true);
		Player->AssignRightGrip(Thruster);
	}
}
void ARaceGameMode::CleanUpSpawns()
{
	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		if (ActorIterator->ActorHasTag(FName("Kill")))
			ActorIterator->Destroy();
	}
}
void ARaceGameMode::SpawnCheckpointTagActors()
{
	CleanUpSpawns();
	
	const AWaypointMarker* Checkpoint = Cast<AWaypointMarker>(CurrentCheckpoint);
	
	// Spawn current checkpoint TagSpawnedPickups
	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{		
		AWaypointMarker* CheckpointWithTags = Cast<AWaypointMarker>(GetCurrentCheckpoint());
		if (CheckpointWithTags)
		{
			if (ActorIterator->ActorHasTag(Checkpoint->GetCheckpointActorTag()))
			{
				FActorSpawnParameters Params;
				AActor* NewActor = GetWorld()->SpawnActor<AActor>(TagSpawnedPickup, ActorIterator->GetActorLocation(), FRotator::ZeroRotator, Params);
				NewActor->Tags.AddUnique(FName("Kill"));
				CurrentCheckpointActors.AddUnique(NewActor);
			}
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

	// Respawn player at checkpoint if available otherwise reload the level
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
		// Waypoints reached before a respawn are reset back to the checkpoint as well
		AWaypointMarker* WP = Cast<AWaypointMarker>(CurrentCheckpoint);
		if (WP && LoadedCourse.Num() > 0)
		{
			for (AWaypointMarker* Waypoint : LoadedCourse)
			{
				Waypoint->DeactivateWaypoint();
			}
			SetCurrentWaypoint(WP->GetWaypointNumber());
			DisplayCurrentWaypoint();
		}
				
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

		if (bRaceSetup)
		{
			GivePlayerThruster(Player);

			AWaypointMarker* Checkpoint = Cast<AWaypointMarker>(CurrentCheckpoint);
			if (Checkpoint)
			{
				
				AHandThruster* Thruster = Cast<AHandThruster>(Player->GetRightHeldObject());
				if (Thruster && Checkpoint->IsAutoHoverCheckpoint())
				{
					Thruster->EnableAutoHover();
				}
			}
		}

		AWaypointMarker* Checkpoint = Cast<AWaypointMarker>(CurrentCheckpoint);
		if (TagSpawnedPickup) SpawnCheckpointTagActors();
		if (Checkpoint) OnPlayerRespawn.Broadcast(Player, Checkpoint->GetCheckpointActorTag());
		else OnPlayerRespawn.Broadcast(Player, "");

	}
}
void ARaceGameMode::EquipRequiredItem(AvrPlayer* PlayerToEquip, TArray<TSubclassOf<AvrPickup>> ItemsToEquip)
{
	if (!PlayerToEquip || ItemsToEquip.Num() == 0 || !PlayerToEquip->GetUtilityBelt() || PlayerToEquip->GetUtilityBelt()->GetEquippedHolsters().Num() == 0) { return; }

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
void ARaceGameMode::CourseFinished()
{
	OnCourseComplete.Broadcast();
	
	if (LoadedCourse.Num() == TimeBetweenWaypoints.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Course completed!"))

		for (int32 i = 0; i < TimeBetweenWaypoints.Num(); i++)
		{
			int32 iCount = i;
			float CheckpointTime = TimeBetweenWaypoints[i];
			UE_LOG(LogTemp, Warning, TEXT("Time to checkpoint %d: %f"), iCount, CheckpointTime)

			OnMessageSend.Broadcast("You won.");

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Course not completed."))
	}

	// If marked as final waypoint, destroy the course head so won't load again
	if (TargetWaypoint->IsFinalWaypoint())
	{
		LoadedCourse[0]->Destroy();
	}
	
	LoadedCourse.Reset();

	// Reset all initial waypoints so courses can be run again, unless it was flagged as a final waypoint
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
	TargetWaypoint = nullptr;

	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		// Waypoint of the correct color are added to array
		AWaypointMarker* Marker = *ActorIter;
		if (Marker->GetCourseColor() == ColorCourseToLoad)
		{
			if (TargetWaypoint == nullptr) TargetWaypoint = Marker; // this needs to be set for OnCourseLoaded broadcast
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

	OnCourseLoaded.Broadcast(GetTargetWaypoint());
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

