// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class AvrProjectile;

UCLASS()
class VR_PROJECT_API ATurret : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATurret();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* Base;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* Turret;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* Barrel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USceneComponent* BarrelPitchPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USphereComponent* RadarZone;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UHealthStats* TurretHealth;

	// Targeting Functions
	UFUNCTION()
	void ScanTripWire(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void ScanForPawns();
	UFUNCTION()
	void TakeAim(FVector AimPoint);
	UFUNCTION()
	void OpenFire();
	UFUNCTION()
	FVector FindFiringSolution();
	UFUNCTION()
	bool LineTraceForPawn(APawn* TargetPawn);

	// Variables
	UPROPERTY(BlueprintReadOnly)
	AActor* ClosestPawn;

	UPROPERTY(BlueprintReadOnly)
	bool bTargetDetectedInLOS = false;

	FTimerHandle ScanInterval_Timer;
	float ScanSpeed = 1.f;

	FTimerHandle FireSolutionInterval_Timer;
	float FireSolutionInterval = 0.08f;

	FTimerHandle FireRate_Timer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	float FireRate = 0.25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming Speed") // Degrees per second
	float TurretYawSpeed = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming Speed") // Degrees per second
	float BarrelPitchSpeed = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	TSubclassOf<AvrProjectile> Ammunition;
	UPROPERTY()
	float AmmoSpeed;

	UPROPERTY(BlueprintReadOnly)
	FVector PredictedPlayerPosition;

	// BP Specific
	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayFireFX();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
