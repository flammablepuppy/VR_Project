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
public:
	virtual void Tick(float DeltaTime);

protected:

	//	VARIABLES
	//

	UPROPERTY()
	AWeaponMag* TargetMagazine;

	//	FUNCTIONS
	//

	UFUNCTION()
	void LoadMag();

public:

	virtual void SnapInitiate(USceneComponent * NewParentComponent, FName SocketName = NAME_None) override;
	virtual void SnapOn() override;

	UFUNCTION()
	void SetTargetMag(AWeaponMag* NewTarget);

};
