// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "SpecialVariables.h"
#include "Components/ActorComponent.h"
#include "HealthStats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FDamageTakenSignature, UHealthStats*, HealthStatsComp, float, Health, float, Damage, 
const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOwnerDied, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOwnerRespawned, AActor*, RespawningActor);


// COMBAT EFFECTS HANDLING 
//////////////////////////////

UENUM(BlueprintType)
enum class EEffectTag : uint8
{
	Tag_Default		UMETA(DisplayName = "Default"),
	Tag_Slow		UMETA(DisplayName = "Slow"),
	Tag_Haste		UMETA(DisplayName = "Haste"),
	Tag_SpeedChange	UMETA(DiaplayName = "Speed Change"),
	Tag_Bleed		UMETA(DisplayName = "Bleed"),
	Tag_Poison		UMETA(DisplayName = "Poison"),
	Tag_Immune		UMETA(DisplayName = "Immune"),
	Tag_Invul		UMETA(DisplayName = "Invulnerable")
};

/**
*	Used by handler to store active effects
*/
USTRUCT(BlueprintType)
struct FCombatEffect
{
	GENERATED_BODY()

		UPROPERTY()
		EEffectTag EffectTag;

	UPROPERTY()
		FTimerHandle EffectHandle;

	// Default constructor
	FCombatEffect()
	{
		EffectTag = EEffectTag::Tag_Default;
		EffectHandle = FTimerHandle::FTimerHandle();
	}

	// Constructor taking parameters
	FCombatEffect(EEffectTag Tag, FTimerHandle Handle)
	{
		EffectTag = Tag;
		EffectHandle = Handle;
	}
};
//////////////////////////////////////////////////

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
	class AvrPlayer * OwningPlayer;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	APlayerController* OwningController;

	UPROPERTY()
	TArray<AvrPickup*> YardSaleDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	float BaseMoveSpeed;

public:	

// PUBLIC FUNCTIONS
/////////////////////

	UFUNCTION()
	void OwnerTakesDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	void SetIsDead(bool NewState);

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintCallable)
	void Respawn();

	UFUNCTION(BlueprintCallable)
	void YardSale(float DroppedItemsLifespan = -1.f);

	UFUNCTION(BlueprintCallable)
	void MemorizePlayerItems(TArray<AvrPickup*>& OutInventory);

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHealth() { return CurrentHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHealth() { return MaximumHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetBaseMoveSpeed() { return BaseMoveSpeed; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsDead() { return bOwnerIsDead; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE TArray<AvrPickup*> GetYardSaleDrop() { return YardSaleDrop; }

	UFUNCTION()
	void SetCurrentHealth(float NewHealth);


// DELEGATES
//////////////

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FDamageTakenSignature DamageTaken;

	UPROPERTY(BlueprintAssignable)
	FOwnerDied OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOwnerRespawned OnRespawn;

	

// Effects Handling
/////////////////////
	UPROPERTY()
	TArray<FCombatEffect> ActiveEffects;

	UFUNCTION(BlueprintCallable)
	bool EffectIsActive(EEffectTag Tag);

	UFUNCTION()
	void PurgeInactiveEffects();

	/**
	* Apply slow to actor
	* @ param Power		Percent slow to apply
	* @ param Duration	Seconds slow applied for
	*/
	UFUNCTION(BlueprintCallable)
	void ApplySlow(float Power, float Duration = -1.f, bool AddNotMultiply = false);
	
	UFUNCTION()
	void RemoveSlow(float Power);

	/**
	* Apply speed buff to actor
	* @ Param Power				Percent buff to apply
	* @ Param Duration			Seconds to apply buff for, < 0 is permanant
	* @ Param AddNotMultiply	True : Power will be added instead of multiplied
	* @ Param ClampMax			Clamp max move speed, values under 0 have no effect
	*/
	UFUNCTION(BlueprintCallable)
	void ApplyHaste(float Power, float Duration = -1.f, bool AddNotMultiply = false, float ClampMax = -1.f);

	UFUNCTION()
	void RemoveHaste(float Power);

	/**
	* Change actor speed to new value
	* @ Param	New speed value
	* @ Param	Duration the speed change lasts, values below zero are permanant
	* @ Param
	*/
	UFUNCTION()
	void SetSpeed(float NewSpeed, float Duration = -1.f);

	/**
	* Apply bleed, damages actor for number of tick
	* @ Param Power		Total damage delt by effect
	* @ Param Duration	The time the damage is distrubuted over
	* @ Param Ticks		Damage ticks per second
	*/	
	UFUNCTION(BlueprintCallable)
	void BleedOn(float Power, float Duration, float Ticks);

	UFUNCTION()
	void BleedTick(FTimerHandle BleedHandle, float TickDamage, float TickInterval);

};
