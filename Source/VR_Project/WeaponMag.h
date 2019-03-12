// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "WeaponMag.generated.h"

class AvrProjectile;

/**
 * 
 */
UCLASS()
class VR_PROJECT_API AWeaponMag : public AvrPickup
{
	GENERATED_BODY()
	
public:
	AWeaponMag();

protected:
	virtual void BeginPlay() override;

	/** Projectile class that is passed to the firing weapon for spawning */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine")
	TSubclassOf<AvrProjectile> LoadedAmmunition;

	/** Maximum capacity of the magazine when fully loaded */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine")
	int32 MaxCapacity = 15;

	/** Initialized in BeginPlay, -1 initializes as a full mag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	int32 CurrentCapacity = -1;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Category = "Getter")
	FORCEINLINE TSubclassOf<AvrProjectile> GetLoadedAmmunition() { return LoadedAmmunition; }
	UFUNCTION(Category = "Getter")
	FORCEINLINE int32 GetMaxCapacity() { return MaxCapacity; }
	UFUNCTION(Category = "Getter")
	FORCEINLINE int32 GetCurrentCapacity() { return CurrentCapacity; }
	UFUNCTION(Category = "Setter")
	void SetCapacity(int32 NewCurrentCapacity);

	UFUNCTION()
	void ExpendCartridge(int32 RoundsExpended = 1);

};
