// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RaceGameMode.generated.h"

class AWaypointMarker;
class AvrPlayer;
class AvrPickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSendMessageSignature, FString, TextToPrint);

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
	float CourseStartTime = 0.f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<float> TimeBetweenWaypoints;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AActor * CurrentCheckpoint;

	UPROPERTY()
	bool bRespawnWithInventory = false;

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
	void HideAllWaypoints();
	   
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
	AActor* GetCurrentCheckpoint() { return CurrentCheckpoint; }

	UFUNCTION(BlueprintCallable)
	void SetRespawnWithInventory(bool NewState);


// DELEGATE
/////////////

	UPROPERTY(BlueprintAssignable)
	FSendMessageSignature OnMessageSend;
};
