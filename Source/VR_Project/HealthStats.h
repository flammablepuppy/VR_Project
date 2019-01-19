// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthStats.generated.h"


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

	UFUNCTION()
	void PlayerDeath();

	UPROPERTY(BlueprintReadOnly)
	bool bOwnerIsDead = false;

public:	
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OwnerTakesDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void DamageFloatingNumber(float Damage);
		
	UPROPERTY(BlueprintReadOnly)
	bool bShowDeathMessage = false;

	UFUNCTION()
	FORCEINLINE float GetCurrentHealth() { return CurrentHealth; }
	UFUNCTION()
	FORCEINLINE float GetMaxHealth() { return MaximumHealth; }

};
