// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "HandThruster.h"
#include "GameFramework/GameMode.h"
#include "RaceGameMode.generated.h"

class AWaypointMarker;
class AvrPlayer;
class AvrPickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSendMessageSignature, FString, TextToPrint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerRespawnNotice, AvrPlayer*, RespawnedPlayer, FName, Tag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCourseLoadedDelegate, AWaypointMarker*, CourseWaypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCourseCompletedDelegate);

/**
 * 
 */
UCLASS()
class VR_PROJECT_API ARaceGameMode : public AGameMode
{
	GENERATED_BODY()

	ARaceGameMode();

protected:
// VARIABLES
///////////////

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	TArray<AWaypointMarker*> LoadedCourse;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<AvrPlayer*> vrPlayers;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 CurrentWaypoint = 0;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AWaypointMarker* TargetWaypoint;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float CourseStartTime = 0.f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<float> TimeBetweenWaypoints;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AActor * CurrentCheckpoint;

	UPROPERTY()
	bool bRespawnWithInventory = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bShouldYardSale = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	bool bRaceSetup = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	bool bFloorIsLava = false;
	
	// Type of Hand Thruster given to player when GivePlayerThruster is called.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AHandThruster> GivenThruster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	TSubclassOf<AActor> TagSpawnedPickup;

	UPROPERTY()
	TArray<AActor*> CurrentCheckpointActors;
	
// FUNCTIONS
//////////////

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void HandlePlayerDeath(AActor* DyingActor);

	UFUNCTION(BlueprintCallable)
	void RespawnPlayers();

	UFUNCTION(BlueprintCallable)
	void EquipRequiredItem(AvrPlayer* PlayerToEquip, TArray<TSubclassOf<AvrPickup>> ItemsToEquip);

	UFUNCTION(BlueprintCallable)
	void GivePlayerThruster(AvrPlayer* Player);

	UFUNCTION()
	void CleanUpSpawns();

	   
public:
// PUBLIC FUNCTIONS
///////////////

	UFUNCTION(BlueprintCallable)
	void CourseFinished();

	UFUNCTION(BlueprintCallable)
	void LoadCourse(FColor ColorCourseToLoad);

	UFUNCTION(BlueprintCallable)
	void DisplayCurrentWaypoint();

	UFUNCTION(BlueprintCallable)
	void SetActiveCheckpoint(AActor * CheckpointActor);

	UFUNCTION(BlueprintPure)
	AActor* GetCurrentCheckpoint() { return CurrentCheckpoint;  }

	UFUNCTION(BlueprintPure)
	AWaypointMarker* GetTargetWaypoint() { return TargetWaypoint; }

	UFUNCTION(BlueprintCallable)
	void SetRespawnWithInventory(bool NewState);

	UFUNCTION(BlueprintCallable)
	void ModeOpenLevel(FString LevelToOpen);

	UFUNCTION(BlueprintCallable)
	void SetShouldYardSale(bool NewState);

	UFUNCTION(BlueprintCallable)
	void SetTargetWaypoint(AWaypointMarker* Waypoint);

	UFUNCTION(BlueprintCallable)
	void SetCurrentWaypoint(int32 WaypointNumber);
	
	UFUNCTION(BlueprintCallable)
	TArray<AWaypointMarker*> GetLoadedCourse() { return LoadedCourse; }
	
	/** Destroys player utility belt and holsters, spawns a non-drop thruster */
	UFUNCTION(BlueprintCallable)
	void PrepPlayerForRace();
	
	UFUNCTION()
	void SpawnCheckpointTagActors();
	
// DELEGATE
/////////////

	UPROPERTY(BlueprintAssignable)
	FSendMessageSignature OnMessageSend;

	UPROPERTY(BlueprintAssignable)
	FPlayerRespawnNotice OnPlayerRespawn;

	UPROPERTY(BlueprintAssignable)
	FCourseLoadedDelegate OnCourseLoaded;

	UPROPERTY(BlueprintAssignable)
	FCourseCompletedDelegate OnCourseComplete;
};

