// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "WeaponMag.generated.h"

class USphereComponent;
class AvrProjectile;
class AMagCartridge;

UCLASS()
class VR_PROJECT_API AWeaponMag : public AvrPickup
{
	GENERATED_BODY()
	
public:
	AWeaponMag();

protected:
	virtual void BeginPlay() override;

	//		COMPONENTS
	//

	/** When a MagCartridge of the CompatibleCartridge class enters this sphere, it loads into the magazine */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* CartridgeLoadSphere;

	//		VARIABLES
	//

	/** Class that will load into the magazine when it enters the CartridgeLoadSphere */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine")
	TSubclassOf<AMagCartridge> CompatibleCartidge;

	/** Maximum capacity of the magazine when fully loaded */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Magazine")
	int32 MaxCapacity = 16;

	/** Initialized in BeginPlay, -1 initializes as a full mag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	int32 CurrentCapacity = -1;

	UPROPERTY()
	AvrPlayer* TargetPlayerForHolstering;


	//		FUNCTIONS
	//

	UFUNCTION()
	void LoadCompatibleCartridge(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void UnPrimeCartridge(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Boolean that prevents a magazine being loaded into a weapon from snapping into a holster when Drop is called prior to the SnapInitiate to the MagazineWell */
	UPROPERTY()
	bool bLoading = false;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SnapOn() override;
	virtual void Drop() override;
	
	/** Bound when loaded into a gun, ensures it is destroyed when gun it's loaded into is destroyed */
	UFUNCTION()
	void Sepeku(AActor* Actor);

	//		GET
	//

	UFUNCTION(Category = "Getter")
	FORCEINLINE TSubclassOf<AMagCartridge> GetCompatibleCartridge() const { return CompatibleCartidge; }

	UFUNCTION(Category = "Getter")
	FORCEINLINE USphereComponent* GetCartridgeLoadSphere() const { return CartridgeLoadSphere; }

	UFUNCTION(Category = "Getter")
	FORCEINLINE int32 GetMaxCapacity() const { return MaxCapacity; }

	UFUNCTION(Category = "Getter")
	FORCEINLINE int32 GetCurrentCapacity() const { return CurrentCapacity; }

	//		SET
	//

	UFUNCTION(Category = "Setter")
	void SetCapacity(int32 NewCurrentCapacity);

	UFUNCTION()
	void ExpendCartridge(int32 RoundsExpended = 1);

	UFUNCTION()
	void SetLoading(bool NewState);

	//		PUBLIC FUNCTIONS
	//

	UFUNCTION()
	void SetMagSearchForHolster(AvrPlayer* PlayerToHolster);

	UFUNCTION()
	void SnapToVacantHolster();

};
