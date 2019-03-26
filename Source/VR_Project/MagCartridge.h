// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "MagCartridge.generated.h"

class AWeaponMag;
class AvrProjectile;

UCLASS()
class VR_PROJECT_API AMagCartridge : public AvrPickup
{
	GENERATED_BODY()
	

public:
	AMagCartridge();
protected:
	virtual void BeginPlay();
public:
	virtual void Tick(float DeltaTime);

protected:

	//	VARIABLES
	//

	UPROPERTY()
	AWeaponMag* TargetMagazine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cartridge Properties")
	TSubclassOf<AvrProjectile> RoundProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cartridge Properties")
	TSubclassOf<AvrPickup> RoundCasing;

	//	FUNCTIONS
	//

	UFUNCTION()
	void LoadMag();

public:

	virtual void SnapInitiate(USceneComponent * NewParentComponent, FName SocketName = NAME_None) override;
	virtual void SnapOn() override;

	UFUNCTION()
	FORCEINLINE TSubclassOf<AvrProjectile> GetProjectile() { return RoundProjectile; }

	UFUNCTION()
	FORCEINLINE TSubclassOf<AvrPickup> GetCasing() { return RoundCasing; }

	UFUNCTION()
	void SetTargetMag(AWeaponMag* NewTarget);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayCartridgeLoad();

};
