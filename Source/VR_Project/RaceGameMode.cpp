// Copyright Aviator Games 2019


#include "RaceGameMode.h"
#include "Engine\World.h"
#include "EngineUtils.h"
#include "WaypointMarker.h"
#include "vrPlayer.h"
#include "HealthStats.h"

ARaceGameMode::ARaceGameMode()
{

}

void ARaceGameMode::BeginPlay()
{
	// Sub to all players OnDeath
	for (TActorIterator<AvrPlayer> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		ActorIter->GetHealthStats()->OnDeath.AddDynamic(this, &ARaceGameMode::CourseFinished);
	}
}

/** End race and calculate times/performance */
void ARaceGameMode::CourseFinished()
{
	if (LoadedCourse.Num() == TimeBetweenWaypoints.Num())
	{
		for (int32 i = 0; i < TimeBetweenWaypoints.Num(); i++)
		{
			int32 iCount = i;
			float CheckpointTime = TimeBetweenWaypoints[i];
			UE_LOG(LogTemp, Warning, TEXT("Time to checkpoint %d: %f"), iCount, CheckpointTime)
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Course Finished!"))

	/** Reset all initial waypoints so courses can be run again */
	for (TActorIterator<AWaypointMarker> ActorIter(GetWorld()); ActorIter; ++ActorIter)
	{
		if (ActorIter->GetWaypointNumber() == 0)
		{
			ActorIter->ActivateWaypoint();
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
			//UE_LOG(LogTemp, Warning, TEXT("Current Waypoint: %d"), CurrentWaypoint)
			Marker->ActivateWaypoint();
		}
	}
	
	if (CurrentWaypoint == LoadedCourse.Num())
	{
		CourseFinished();
	}
}

