// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "SigPistol.generated.h"

class USkeletalMeshComponent;
class AvrProjectile;
class AWeaponMag;
class USphereComponent;
class AMagCartridge;
class AvrHolster;
class USceneComponent;
class UMotionController;

UCLASS()
class VR_PROJECT_API ASigPistol : public AvrPickup
{
	GENERATED_BODY()

public:
	ASigPistol();

protected:
	virtual void BeginPlay() override;

	//		VARIABLES
	//

	/** The parent class static mesh "PickupMesh" is used for collision, this mesh has no collision but has all the sockets for effects and animations */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* PistolMesh;

	/** When a compatible magazine is moved into this sphere, it will be loaded into the weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* MagazineLoadSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* WeaponForegrip;

	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	bool bSlideBack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties: Automatic Fire")
	bool bAutomaticFire = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties: Automatic Fire")
	float AutoCooldown = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon Properties: Two Handed")
	bool bTwoHandEnabled = false;

	FTimerHandle AutoCooldown_Timer;

	/** Pointer to currently loaded magazine */
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	AWeaponMag* LoadedMagazine;
	
	UPROPERTY()
	AvrHolster* TargetVacantHolster;

	UPROPERTY()
	AWeaponMag* DroppedMagWaitingToHolster;

	/** If magazine is beyond this distance from a vacant holster after being ejected, it won't auto attach */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine Properties")
	float TooFarDistance = 300.f;

	/** Time after ejecting a magazine before it seeks a vacant holster */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine Properties")
	float DropToHolsterTime = 0.55f;

	/** Projectile currently in chamber, used to spawn projectile on trigger pull */
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	TSubclassOf<AMagCartridge> ChamberedRound;

	/** If true, the weapon will spawn with a full magazine on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol Properties")
	bool bSpawnsLoaded = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol Properties", meta = (EditCondition = "bSpawnsLoaded"))
	bool bSpawnsChambered = true;

	/** Magazine that spawns with weapon when bSpawnsLoaded is true */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine Properties")
	TSubclassOf<AWeaponMag> CompatibleMagazine;

	/** Capacity of spawned magazine, -1 defaults to MaxCapacity for the magazine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Properties")
	int32 StarterCapacity = -1;

	//		FUNCTIONS
	//

	/** Function that handles the movement of a detected magazine into the magwell and setting it as the LoadedMagazine */
	UFUNCTION()
	void MagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void ForegripSub(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void GrabForegrip(UMotionControllerComponent* RequestingController);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void TriggerPulled(float Value) override;
	bool bTriggerPulled = false;
	virtual void TopPushed() override;
	//virtual void TopReleased() override; NOT IN USE
	virtual void BottomPushed() override;
	//virtual void BottomReleased() override; NOT IN USE
	virtual void Drop() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayMagLoad();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayHammer();

protected:
	UFUNCTION()
	void DischargeRound();
	UFUNCTION()
	void AttemptCharge();

	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void PlayFireFX();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void PlaySlideBack();
	UFUNCTION(BlueprintImplementableEvent, Category = "BP Functions")
	void PlaySlideForward();
	
};
