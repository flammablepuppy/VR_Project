// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AvrPickup;
class UParticleSystemComponent;

UCLASS()
class VR_PROJECT_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ACheckpoint();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;

protected:

// COMPONENTS
///////////////

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* CheckpointMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* CheckpointSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UParticleSystemComponent* CheckpointParticleSystem;

// VARIABLES
//////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Effects")
	TSubclassOf<AvrPickup> RequiredItem;

// FUNCTIONS
//////////////

	UFUNCTION()
	void ActivateCheckpoint(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:

// PUBLIC FUNCTIONS
/////////////////////

	UFUNCTION(BlueprintPure)
	UStaticMeshComponent* GetCheckpointMesh() { return CheckpointMesh; }

	UFUNCTION(BlueprintPure)
	USphereComponent* GetCheckpointSphere() { return CheckpointSphere; }

	UFUNCTION(BlueprintPure)
	TSubclassOf<AvrPickup> GetRequiredItem() { return RequiredItem; }
};
