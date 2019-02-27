// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "vrPickup.h"
#include "SigPistol.generated.h"

class USkeletalMeshComponent;
class AvrProjectile;
class AWeaponMag;

UCLASS()
class VR_PROJECT_API ASigPistol : public AvrPickup
{
	GENERATED_BODY()

public:
	ASigPistol();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* PistolMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	bool bSlideBack = false;
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	bool bRoundChambered = true;
	
	UPROPERTY(BlueprintReadOnly, Category = "Pistol Properties")
	AWeaponMag* LoadedMagazine;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pistol Properties")
	TSubclassOf<AvrProjectile> AmmoType;

public:
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
		

};
