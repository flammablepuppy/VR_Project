// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaypointMarker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaypointCollected);

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

// VARIABLES
//////////////

	/** Used by GameMode to determine which waypoint should be active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint Properties")
	int32 WaypointNumber = 0;

	/** Used by GameMode to differntiate between multiple routes in a level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint Properties")
	FColor CourseColor = FColor::Green;

	/** If true, player will respawn at this checkpoint upon death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint Properties")
	bool bFunctionsAsCheckpoint = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Waypoint Properties")
	class USoundBase* CollectionSound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bWaypointIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint Properties")
	TArray<TSubclassOf<AvrPickup>> ItemsToSpawn;

// FUNCTIONS
//////////////

	UFUNCTION(BlueprintCallable)
	void WaypointReached(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

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

// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable)
	FWaypointCollected OnCollected;

};
