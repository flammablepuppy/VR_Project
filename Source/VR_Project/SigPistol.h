// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "SigPistol.generated.h"

class USkeletalMeshComponent;
class AvrProjectile;
class AWeaponMag;
class USphereComponent;

UCLASS()
class VR_PROJECT_API ASigPistol : public AvrPickup
{
	GENERATED_BODY()

public:
	ASigPistol();

protected:
	virtual void BeginPlay() override;

	/** The parent class static mesh "PickupMesh" is used for collision, this mesh has no collision but has all the sockets for effects and animations */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* PistolMesh;

	/** When a compatible magazine is moved into this sphere, it will be loaded into the weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* MagazineLoadSphere;

	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	bool bSlideBack = false;

	/** Pointer to currently loaded magazine */
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	AWeaponMag* LoadedMagazine;

	/** Set true when magazine has entered the MagazineLoadSphere and is animating to a seated position */
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	bool bMagInTransit = false;

	/** Time it takes for the magazine to snap into the weapon after entering the MagazineLoadSphere */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties")
	float MagazineLoadTime = 0.15f;

	/** Projectile currently in chamber, used to spawn projectile on trigger pull */
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	TSubclassOf<AvrProjectile> ChamberedRound;

	/** If true, the weapon will spawn with a full magazine on BeginPlay */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties")
	bool bSpawnsLoaded = true;

	/** Magazine that spawns with weapon when bSpawnsLoaded is true */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties")
	TSubclassOf<AWeaponMag> StarterMagazine;

	/** Capacity of spawned magazine, -1 defaults to MaxCapacity for the magazine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pistol Properties")
	int32 StarterCapacity = -1;

	/** Function that handles the movement of a detected magazine into the magwell and setting it as the LoadedMagazine */
	UFUNCTION()
	void MagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void Drop() override;
	virtual void TriggerPulled(float Value) override;
	bool bTriggerPulled = false;
	virtual void TopPushed() override;
	virtual void TopReleased() override;
	virtual void BottomPushed() override;
	virtual void BottomReleased() override;

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

	UFUNCTION()
	void MoveMagToWell();
	UFUNCTION()
	void AttachMag();
	UFUNCTION()
	void DropMag();

		

};
