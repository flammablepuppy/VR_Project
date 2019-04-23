// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthStats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FDamageTakenSignature, UHealthStats*, HealthStatsComp, float, Health, float, Damage, 
const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOwnerDied, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOwnerRespawned, AActor*, RespawningActor);

class AvrPlayer;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VR_PROJECT_API UHealthStats : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthStats();

protected:
	virtual void BeginPlay() override;

// VARIABLES
//////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	float MaximumHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Default Values")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	bool bOwnerIsDead = false;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AvrPlayer * OwningPlayer;

	UPROPERTY()
	TArray<AvrPickup*> YardSaleDrop;

public:	

// PUBLIC FUNCTIONS
/////////////////////

	UFUNCTION()
	void OwnerTakesDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void SetIsDead(bool NewState);

	UFUNCTION()
	void Die();

	UFUNCTION()
	void Respawn();

	/** Drop all held items and items in holsters, returns array of dropped items */
	UFUNCTION()
	void YardSale();

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHealth() { return CurrentHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHealth() { return MaximumHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsDead() { return bOwnerIsDead; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AvrPickup*> GetYardSaleDrop() { return YardSaleDrop; }


// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDamageTakenSignature DamageTaken;

	UPROPERTY(BlueprintAssignable)
	FOwnerDied OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOwnerRespawned OnRespawn;

};
