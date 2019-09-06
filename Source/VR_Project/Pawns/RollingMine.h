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

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

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

	UFUNCTION(Category = "Searching")
	void Search(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Category = "Searching")
	void SeekClosestPlayer();
	class AvrPlayer* TargetPlayer;

	FVector FindTargetDirection();
	float FindMoveForce();
	void MoveToTarget();
	bool CheckIsAirbourne();

	// TODO
	/** Sound played while moving around */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	class USoundCue* RollingSound;
	FTimerHandle RollLoop_Timer;
	float RollLoopDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float AllowableAngle = 5.f;

	/** Max cm/s acceleration force */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float MaxMoveForce = 3200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float BrakeOnset = 0.95f;

	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float BrakingForce = 6400.f;

	/** Max cm/s allowable speed */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float MoveMaxSpeed = 900.f;

	/** How often to check if the tracked player is still in the mine's line of sight */
	UPROPERTY(EditDefaultsOnly, Category = "Searching")
	float LOSTickInterval = 1.f;
	FTimerHandle LOSTick_Timer;

// EXPLOSION
//////////////

	UFUNCTION(Category = "Explosion")
	void Explode(AActor* DyingActor);

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	USoundCue* ExplosionSound;

	bool bHasExploded = false;

	/** Damage applied to all actors within ExplosionSphere unpon death */
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionDamage = 65.f;

// STAB
/////////

	UFUNCTION()
	void SpikeStab(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category = "Stab")
	USoundCue* StabSound;

	/** Damage dealt when RollingMine impacts an actor */
	UPROPERTY(EditDefaultsOnly, Category = "Stab")
	float SpikeDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stab")
	float ImpulsePower = 100.f;

	/** Minimum interval between damage ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Stab")
	float SpikeCooldownDuration = 1.f;
	FTimerHandle SpikeCooldown_Timer; 
	
public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UStaticMeshComponent* GetMesh() { return MineMesh; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UHealthStats* GetHealthStats() { return MineHealth; }

};
