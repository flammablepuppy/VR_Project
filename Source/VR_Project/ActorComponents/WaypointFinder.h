// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "WaypointMarker.h"
#include "GameFramework/Actor.h"
#include "WaypointFinder.generated.h"

UCLASS()
class VR_PROJECT_API AWaypointFinder : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaypointFinder();
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ArrowMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* ArrowMaterial;

	UFUNCTION()
	void Point();

	UFUNCTION()
	void HandleCourseLoaded(AWaypointMarker* CourseWaypoint);

	UFUNCTION()
	void HandleCourseCompleted();

};
