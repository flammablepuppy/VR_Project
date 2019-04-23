// Copyright Aviator Games 2019


#include "Checkpoint.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "vrPlayer.h"
#include "RaceGameMode.h"

// Sets default values
ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = true;

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("Checkpoint Mesh");
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	CheckpointMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CheckpointMesh;

	CheckpointSphere = CreateDefaultSubobject<USphereComponent>("Checkpoint Sphere");
	CheckpointSphere->SetupAttachment(RootComponent);

	CheckpointParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("Checkpoint Particle System");
	CheckpointParticleSystem->SetupAttachment(RootComponent);

}
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
	CheckpointSphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::ActivateCheckpoint);
}
void ACheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/**
*
*/

/** Sets this Checkpoint as the GameMode's CurrentCheckpoint */
void ACheckpoint::ActivateCheckpoint(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check that vrPlayer overlapped, the GameMode supports checkpoints and that this checkpoint isn't already set as the active checkpoint
	AvrPlayer* Player = Cast<AvrPlayer>(OtherActor);
	ARaceGameMode* Mode = Cast<ARaceGameMode>(GetWorld()->GetAuthGameMode());
	if (Player && Mode && Mode->GetCurrentCheckpoint() != this)
	{
		Mode->SetActiveCheckpoint(this);
	}
}
