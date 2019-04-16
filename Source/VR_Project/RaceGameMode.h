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

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	TArray<AWaypointMarker*> LoadedCourse;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 CurrentWaypoint = 0;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float CourseStartTime = 0.f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<float> TimeBetweenWaypoints;

public:
// PUBLIC FUNCTIONS
///////////////

	UFUNCTION(BlueprintCallable)
	void CourseFinished();

	UFUNCTION(BlueprintCallable)
	void LoadCourse(FColor ColorCourseToLoad);

	UFUNCTION(BlueprintCallable)
	void DisplayCurrentWaypoint();

};
