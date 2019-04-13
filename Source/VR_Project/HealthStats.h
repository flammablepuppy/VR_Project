// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthStats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FDamageTakenSignature, UHealthStats*, HealthStatsComp, float, Health, float, Damage, 
const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOwnerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOwnerRespawned);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VR_PROJECT_API UHealthStats : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthStats();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	float MaximumHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Default Values")
	float CurrentHealth;

	UPROPERTY()
	float Currency = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay Mechanics")
	FVector CheckpointLocation = FVector::ZeroVector;

	UFUNCTION()
	void PlayerDeath();

public:	
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OwnerTakesDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDamageTakenSignature DamageTaken;

	UPROPERTY(BlueprintReadOnly)
	bool bOwnerIsDead = false;

	UFUNCTION()
	FORCEINLINE float GetCurrentHealth() { return CurrentHealth; }
	UFUNCTION()
	FORCEINLINE float GetMaxHealth() { return MaximumHealth; }
	UFUNCTION()
	FORCEINLINE bool CheckIsDead() { return bOwnerIsDead; }

	UPROPERTY(BlueprintAssignable)
	FOwnerDied OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOwnerRespawned OnRespawn;

	UFUNCTION()
	void SetCheckpointLocation(FVector NewLocation);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrency() { return Currency; }

	UFUNCTION(BlueprintCallable)
	void AdjustCurrency(float CurrencyAdjustment = 1.f);

};
