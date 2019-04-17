// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthStats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FDamageTakenSignature, UHealthStats*, HealthStatsComp, float, Health, float, Damage, 
const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOwnerDied, AActor*, DyingActor);

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

	UPROPERTY()
	float Currency = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay Mechanics")
	FVector CheckpointLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	bool bOwnerIsDead = false;

// FUNCTIONS
//////////////

	UFUNCTION()
	void HandleDeath();

public:	

// PUBLIC FUNCTIONS
/////////////////////

	UFUNCTION()
	void OwnerTakesDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	FORCEINLINE float GetCurrentHealth() { return CurrentHealth; }

	UFUNCTION()
	FORCEINLINE float GetMaxHealth() { return MaximumHealth; }

	UFUNCTION()
	FORCEINLINE bool GetIsDead() { return bOwnerIsDead; }

	UFUNCTION()
	FORCEINLINE FVector GetCheckpoint() { return CheckpointLocation; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrency() { return Currency; }

	UFUNCTION(BlueprintCallable)
	void AdjustCurrency(float CurrencyAdjustment = 1.f);

	UFUNCTION()
	void SetCheckpointLocation(FVector NewLocation);

	UFUNCTION()
	void SetIsDead(bool NewState);

// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDamageTakenSignature DamageTaken;

	UPROPERTY(BlueprintAssignable)
	FOwnerDied OnDeath;

};
