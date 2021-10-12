// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaypointMarker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaypointCollected);

class AvrPickup;

UCLASS()
class VR_PROJECT_API AWaypointMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaypointMarker();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;

protected:
// COMPONENTS
///////////////

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Waypoint Components")
	class USceneComponent* WaypointRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Waypoint Components")
	class UStaticMeshComponent* WaypointMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Waypoint Components")
	class USphereComponent* WaypointCollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Waypoint Components")
	UAudioComponent* CollectingTone;

// VARIABLES
//////////////

	/** Used by GameMode to determine which waypoint should be active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	int32 WaypointNumber = 0;

	/** Used by GameMode to differntiate between multiple routes in a level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	FColor CourseColor = FColor::Green;

	/** If true, player will respawn at this checkpoint upon death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	bool bFunctionsAsCheckpoint = false;

	/** Sound played when overlapped by player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Course Properties")
	class USoundBase* CollectionSound;

	/** Flag for Race Game Mode */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bWaypointIsActive = false;

	/** Flag for Race Game Mode, true does not reload 0 waypoints after completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	bool bFinalWaypoint = false;

	/** Items to spawn when player respawns at this checkpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	TArray<TSubclassOf<AvrPickup>> ItemsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties", meta = (EditCondition = "WaypointNumber == 0"))
	bool bActivatePointerOnBeginPlay = false;
		
// FUNCTIONS
//////////////

	UFUNCTION(BlueprintCallable)
	void WaypointReached(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(BlueprintCallable)
	void WaypointLeft(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void AdvanceCourse(AActor* CollectingActor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	bool bTimedCollection = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Properties")
	float CheckpointCollectionTime = 1.5f;
	
	FTimerHandle Collection_Timer;

	// Used in tick for collection tone
	UPROPERTY(BlueprintReadOnly)
	bool bIsCollecting = false;
	float CollectingPitchChange = 1.f;
		
public:
// PUBLIC FUNCTIONS
/////////////////////

	UFUNCTION(BlueprintCallable)
	void ActivateWaypoint();

	UFUNCTION(BlueprintCallable)
	void DeactivateWaypoint();

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetWaypointIsActive() { return bWaypointIsActive; } 

	UFUNCTION(BlueprintPure)
	FORCEINLINE FColor GetCourseColor() { return CourseColor; } 

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetWaypointNumber() { return WaypointNumber; }  

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<TSubclassOf<AvrPickup>> GetItemsToSpawn() { return ItemsToSpawn; }  

	UFUNCTION(BlueprintPure)
	FORCEINLINE UStaticMeshComponent* GetWaypointMesh() { return WaypointMesh; } 

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsFinalWaypoint() { return bFinalWaypoint; }

// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable)
	FWaypointCollected OnCollected;

};
