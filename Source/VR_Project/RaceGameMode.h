// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RaceGameMode.generated.h"

class AWaypointMarker;

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

	UPROPERTY()
	TArray<AWaypointMarker*> LoadedCourse;

	UPROPERTY()
	int32 CurrentWaypoint = 0;

	UPROPERTY()
	float CourseStartTime = 0.f;

	UPROPERTY()
	TArray<float> TimeBetweenWaypoints;

// FUNCTIONS
///////////////

	UFUNCTION()
	void CourseFinished();

public:
// PUBLIC FUNCTIONS
///////////////

	UFUNCTION()
	void LoadCourse(FColor ColorCourseToLoad);

	UFUNCTION()
	void DisplayCurrentWaypoint();
	
};
