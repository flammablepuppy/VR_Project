// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RollingMine.generated.h"

UCLASS()
class VR_PROJECT_API ARollingMine : public APawn
{
	GENERATED_BODY()

public:
	ARollingMine();

protected:
	virtual void BeginPlay() override;

// COMPONENTS
///////////////

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MineMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* TriggerSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* ExplosionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UHealthStats* MineHealth;


// SEARCHING AND SEEKING
//////////////////////////

	UFUNCTION()
	void Search(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void SeekClosestPlayer();
	class AvrPlayer* TargetPlayer;

	UFUNCTION()
	void MoveToTarget();

	/** Force applied to rolling mine in the direction of TargetPlayer */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float MoveForce = 0.50f;

	/** Max cm/s allowable speed */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float MoveMaxSpeed = 1000.f;

	/** How often to check if the tracked player is still in the mine's line of sight */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float LOSTickInterval = 1.f;
	FTimerHandle LOSTick_Timer;


// EXPLOSION
//////////////

	UFUNCTION()
	void Explode(AActor* DyingActor);

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	class USoundCue* ExplosionSound;

	UPROPERTY(VisibleAnywhere, Category = "Explosion")
	bool bHasExploded = false;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionDamage = 65.f;
	

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
