// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FloatingMine.generated.h"

UCLASS()
class VR_PROJECT_API AFloatingMine : public APawn
{
	GENERATED_BODY()

public:
	AFloatingMine();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	//		COMPONENTS
	//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* MineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USphereComponent* ScanRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* BlastRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UHealthStats* HealthStats;

	//		VARIABLES
	//

	/** Meters per second acceleration rate when homing toward target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mine Properties")
	float MineAcceleration = 25.f;

	/** Terminal velocity of mine when homing toward target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mine Properties")
	float MineTopSpeed = 15.f;

	/** Damage passed to all actors in BlastRadius when mine explodes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mine Properties")
	float MineDamage = 35.f;

	UPROPERTY(BlueprintReadOnly)
	class AvrPlayer* TargetPlayer;

	FVector OldVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine Properties")
	bool bTrackingOn = true;

	bool bScanOn = false;
	bool bHasTarget = false;

	//		FUNCTIONS
	//

	UFUNCTION()
	void OverlapScan(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void ScanForTargets();

	UFUNCTION()
	void HomeTowardTarget();

	UFUNCTION()
	void Explode(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(BlueprintCallable)
	void SetTrackingOn(bool NewState);
};
