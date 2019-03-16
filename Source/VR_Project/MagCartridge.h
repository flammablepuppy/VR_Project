// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "MagCartridge.generated.h"

class AWeaponMag;

UCLASS()
class VR_PROJECT_API AMagCartridge : public AvrPickup
{
	GENERATED_BODY()
	

public:
	AMagCartridge();

protected:
	virtual void BeginPlay();

	//	VARIABLES
	//

	/** Seconds it takes cartrige to load into compatible magazine after entering the magazine'LoadSphere */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cartridge")
	float LoadSpeed = 0.05f;

	UPROPERTY()
	AWeaponMag* TargetMagazine;

	UPROPERTY()
	bool bLoading = false;

	//	FUNCTIONS
	//

	UFUNCTION()
	void MoveToMag();

	UFUNCTION()
	void LoadMag();

public:
	virtual void Tick(float DeltaTime);

	UFUNCTION()
	void SetTargetMag(AWeaponMag* NewTarget);

	UFUNCTION()
	void LoadCartridge();

};
