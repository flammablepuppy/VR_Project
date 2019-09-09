// Copyright Aviator Games 2019

#include "HealthStats.h"
#include "GameFramework/Actor.h"
#include "vrPlayer.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "vrPickup.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "vrBelt.h"
#include "vrHolster.h"
#include "TimerManager.h"

UHealthStats::UHealthStats()
{
	PrimaryComponentTick.bCanEverTick = false;

}
void UHealthStats::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaximumHealth;
	
	OwningPlayer = Cast<AvrPlayer>(GetOwner());
	if (OwningPlayer) OwningController = Cast<APlayerController>(OwningPlayer->GetController());
	
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthStats::OwnerTakesDamage);

}

/**
*
*/

void UHealthStats::OwnerTakesDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage > 0.f || Damage < 0.f)
	{
		DamageTaken.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);

		if (InstigatedBy)
		{
			AvrPlayer* PlayerCausingDamage = Cast<AvrPlayer>(InstigatedBy->GetPawn());
			if (PlayerCausingDamage)
			{
				PlayerCausingDamage->SendCombatText(DamageCauser->GetActorLocation(), Damage, InstigatedBy->GetPawn()->GetActorLocation());
			}
		}

		if (CurrentHealth <= 0.f && !bOwnerIsDead)
		{
			bOwnerIsDead = true;

			if (OnDeath.IsBound())
			{
				OnDeath.Broadcast(GetOwner());
			}
			else
			{
				GetOwner()->Destroy();
			}

			AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
			if (vrPlayerOwner)
			{
				Die();
			}
		}
	}
}
void UHealthStats::Die()
{
	AvrPlayer* Owner = Cast<AvrPlayer>(GetOwner());
	Owner->DisableInput(Cast<APlayerController>(Owner->GetController()));
	Owner->GetMovementComponent()->StopActiveMovement();
	Owner->SetImpactDamageEnabled(false);
	//YardSale();

}
void UHealthStats::SetIsDead(bool NewState)
{
	bOwnerIsDead = NewState;
}
void UHealthStats::Respawn()
{
	OwningPlayer->EnableInput(OwningController);
	SetCurrentHealth(MaximumHealth);
	bOwnerIsDead = false;

	FTimerHandle EnableDamage_Handle;
	FTimerDelegate EnableDamageDelegate;
	EnableDamageDelegate.BindUFunction(OwningPlayer, FName("SetImpactDamageEnabled"), true);
	GetWorld()->GetTimerManager().SetTimer(EnableDamage_Handle, EnableDamageDelegate, 0.5f, false);

	OnRespawn.Broadcast(GetOwner());

}

// TODO Should be handled on the vrPlayer or UtilityBelt
void UHealthStats::YardSale(float DroppedItemsLifespan)
{
	if (DroppedItemsLifespan > 0.f)
	{
		TArray<AvrPickup*> PlayerInventory;
		MemorizePlayerItems(PlayerInventory);
		for (AvrPickup* Item : PlayerInventory)
		{
			if (Item)
			{
				Item->SetLifeSpan(DroppedItemsLifespan);

				// TODO: Make a bool for wether or not dropped items can be recovered
				Item->SetPickupEnabled(false);
				// Weapon magazines were not destroying, this attempts and fails to take care of that TODO TODO TODO
				TArray<AActor*> Children;
				Item->GetAttachedActors(Children);
				for (AActor* Child : Children)
				{
					if (Child)
					{
						Child->SetLifeSpan(DroppedItemsLifespan);
					}
				}
			}
		}
	}

	TArray<AvrHolster*> Holsters = OwningPlayer->GetUtilityBelt()->GetEquippedHolsters();
	for (AvrHolster* Holster : Holsters)
	{
		Holster->DropHolsteredItem();
	}

	if (OwningPlayer->GetLeftHeldObject())
	{
		AvrPickup* RememberLeft = OwningPlayer->GetLeftHeldObject();
		RememberLeft->SetSeeksHolster(false);
		RememberLeft->Drop();
		RememberLeft->SetSeeksHolster(true);
	}

	if (OwningPlayer->GetRightHeldObject())
	{
		AvrPickup* RememberRight = OwningPlayer->GetRightHeldObject();
		RememberRight->SetSeeksHolster(false);
		RememberRight->Drop();
		RememberRight->SetSeeksHolster(true);
	}
}
void UHealthStats::MemorizePlayerItems(TArray<AvrPickup*>& OutInventory)
{
	OutInventory.Reset();

	TArray<AvrHolster*> Holsters = OwningPlayer->GetUtilityBelt()->GetEquippedHolsters();
	for (AvrHolster* Holster : Holsters)
	{
		OutInventory.AddUnique(Holster->GetHolsteredItem());
	}

	OutInventory.AddUnique(OwningPlayer->GetLeftHeldObject());
	OutInventory.AddUnique(OwningPlayer->GetRightHeldObject());
}

// SETTERS
////////////
void UHealthStats::SetCurrentHealth(float NewHealth)
{
	CurrentHealth = NewHealth;
}

// EFFECTS HANDLER 
////////////////////

bool UHealthStats::EffectIsActive(EEffectTag Tag)
{
	for (FCombatEffect Effect : ActiveEffects)
	{
		if (Effect.EffectTag == Tag && GetWorld()->GetTimerManager().IsTimerActive(Effect.EffectHandle)) return true;
	}

	return false;
}

void UHealthStats::PurgeInactiveEffects()
{
	int32 NumActiveEffects = 0;
	for (FCombatEffect Effect : ActiveEffects)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(Effect.EffectHandle))
		{
			NumActiveEffects++;
		}
	}

	if (NumActiveEffects == 0)
	{
		ActiveEffects.Empty();
	}
}

void UHealthStats::ApplySlow(float Power, float Duration, bool AddNotMultiply)
{
	// Check for MoveComp and if they're immune
	UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp && !EffectIsActive(EEffectTag::Tag_Immune))
	{
		// Apply the gameplay effect
		float NewSpeed = 0.f;
		if (AddNotMultiply) NewSpeed = -(MoveComp->MaxWalkSpeed + Power);
		else NewSpeed = MoveComp->MaxWalkSpeed * (1.f - Power);
		MoveComp->MaxWalkSpeed = NewSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("%s speed multiplied by %f. New speed: %f."), *GetOwner()->GetName(), Power, NewSpeed)

		for (FCombatEffect Effect : ActiveEffects)
		{
			// Reverse haste effects immediately
			if (Effect.EffectTag == EEffectTag::Tag_Haste)
			{
				GetWorld()->GetTimerManager().SetTimer(Effect.EffectHandle, 1.f, false, 1.f);
			}
		}

		// Create and set timer to reverse effect
		FTimerHandle Slow_Timer;
		FTimerDelegate Slow_Delegate;
		Slow_Delegate.BindUFunction(this, "UndoSlow", Power);
		GetWorld()->GetTimerManager().SetTimer(Slow_Timer, Slow_Delegate, Duration, false);

		// Make effect visible to other potential effects
		FCombatEffect NewEffect = FCombatEffect(EEffectTag::Tag_Slow, Slow_Timer);
		ActiveEffects.Add(NewEffect);
	}
	
}

void UHealthStats::RemoveSlow(float Power)
{
	UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		// Reverse gameplay effect
		float NewSpeed = MoveComp->MaxWalkSpeed / (1.f - Power);
		MoveComp->MaxWalkSpeed = NewSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("%s speed divided by %f. New speed: %f."), *GetOwner()->GetName(), Power, NewSpeed)
		
		PurgeInactiveEffects();
	}

}

void UHealthStats::ApplyHaste(float Power, float Duration, bool AddNotMultiply, float ClampMax)
{
	// Check for MoveComp
	UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		// Apply the gameplay effect
		float NewSpeed = 0.f;
		if (AddNotMultiply) NewSpeed = MoveComp->MaxWalkSpeed + Power;
		else NewSpeed = MoveComp->MaxWalkSpeed * (1.f + Power);

		if (ClampMax > 0 &&
			NewSpeed < ClampMax &&
			MoveComp->MaxWalkSpeed + Power < ClampMax)
		{
			MoveComp->MaxWalkSpeed = NewSpeed;
		}
		else MoveComp->MaxWalkSpeed = ClampMax;
		//UE_LOG(LogTemp, Warning, TEXT("%s speed multiplied by %f. New speed: %f."), *GetOwner()->GetName(), Power, NewSpeed)

		// Create and set timer to reverse effect
		FTimerHandle Haste_Timer;
		FTimerDelegate Haste_Delegate;
		Haste_Delegate.BindUFunction(this, "RemoveHaste", Power);
		GetWorld()->GetTimerManager().SetTimer(Haste_Timer, Haste_Delegate, Duration, false);

		// Make effect visible to other potential effects
		FCombatEffect NewEffect = FCombatEffect(EEffectTag::Tag_Haste, Haste_Timer);
		ActiveEffects.Add(NewEffect);
	}

}

void UHealthStats::RemoveHaste(float Power)
{
	UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		// Reverse gameplay effect
		float NewSpeed = MoveComp->MaxWalkSpeed / (1.f - Power);
		MoveComp->MaxWalkSpeed = NewSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("%s speed divided by %f. New speed: %f."), *GetOwner()->GetName(), Power, NewSpeed)

		PurgeInactiveEffects();
	}

}

void UHealthStats::SetSpeed(float NewSpeed, float Duration)
{
	UCharacterMovementComponent* MoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	if (MoveComp)
	{
		// Interrupt all slow and haste effects
		//for (FCombatEffect Effect : ActiveEffects)
		//{
		//	if (Effect.EffectTag == EEffectTag::Tag_Slow)
		//	{
		//		GetWorld()->GetTimerManager().ClearTimer(Effect.EffectHandle);
		//	}

		//}
		//for (FCombatEffect Effect : ActiveEffects)
		//{
		//	if (Effect.EffectTag == EEffectTag::Tag_Haste)
		//	{
		//		GetWorld()->GetTimerManager().ClearTimer(Effect.EffectHandle);
		//	}

		//}

		MoveComp->MaxWalkSpeed = NewSpeed;
		float SpeedCheck = MoveComp->MaxWalkSpeed;
		//UE_LOG(LogTemp, Warning, TEXT("%s speed set to %f | Move speed actual: %f"), *GetOwner()->GetName(), NewSpeed, SpeedCheck)

		if (Duration > 0.f)
		{
			// Create and set timer to reverse effect
			FTimerHandle Slow_Timer;
			FTimerDelegate Slow_Delegate;
			Slow_Delegate.BindUFunction(this, "RemoveSlow", NewSpeed);
			GetWorld()->GetTimerManager().SetTimer(Slow_Timer, Slow_Delegate, Duration, false);

			// Make effect visible to other potential effects
			FCombatEffect NewEffect = FCombatEffect(EEffectTag::Tag_SpeedChange, Slow_Timer);
			ActiveEffects.Add(NewEffect);
		}
	}

	PurgeInactiveEffects();

}

void UHealthStats::BleedOn(float Power, float Duration, float Ticks)
{
	UE_LOG(LogTemp, Warning, TEXT("BLEED BEGINS"))

	// Determine variables from parameters for Bleed_Delegate
	float DamagePerTick = Power / Duration / Ticks;
	float TickTime = 1.f / Ticks;

	// Create handle for BleedTick to check
	FTimerHandle Bleed_Handle;
	GetWorld()->GetTimerManager().SetTimer(Bleed_Handle, Duration, false);

	// Create handle for BleedTick to fire
	FTimerHandle BleedTick_Handle;
	FTimerDelegate BleedTick_Delegate;
	BleedTick_Delegate.BindUFunction(this, "BleedTick", Bleed_Handle, DamagePerTick, TickTime);
	GetWorld()->GetTimerManager().SetTimer(BleedTick_Handle, BleedTick_Delegate, TickTime, false);

	// Make effect visible to other potential effects
	FCombatEffect NewEffect = FCombatEffect(EEffectTag::Tag_Bleed, Bleed_Handle);
	ActiveEffects.Add(NewEffect);

	bool DurRegisters = GetWorld()->GetTimerManager().IsTimerActive(Bleed_Handle);
	bool TickReg = GetWorld()->GetTimerManager().IsTimerActive(Bleed_Handle);
	UE_LOG(LogTemp, Warning, TEXT("Bleed reporting on: %s"), DurRegisters ? TEXT("True") : TEXT("False"))
	UE_LOG(LogTemp, Warning, TEXT("Bleed Tick reporting on: %s"), TickReg ? TEXT("True") : TEXT("False"))

}

void UHealthStats::BleedTick(FTimerHandle BleedHandle, float TickDamage, float TickInterval)
{
	UE_LOG(LogTemp, Warning, TEXT("BLEED ticks"))

	// If bleed duration has not finished, deal bleed damage
	if (GetWorld()->GetTimerManager().IsTimerActive(BleedHandle))
	{
		FDamageEvent BleedEvent;
		OwningPlayer->TakeDamage(TickDamage, BleedEvent, nullptr, nullptr);
		UE_LOG(LogTemp, Warning, TEXT("%f bleed damage!"), TickDamage)

		// Create new handle for next BleedTick to fire
		FTimerHandle BleedTick_Handle;
		FTimerDelegate BleedTick_Delegate;
		BleedTick_Delegate.BindUFunction(this, "BleedTick", BleedHandle, TickDamage, TickInterval);
		GetWorld()->GetTimerManager().SetTimer(BleedTick_Handle, BleedTick_Delegate, TickInterval, false);

	}
	//  Effect has ended, clear the bleed damage tick
	else
	{
		PurgeInactiveEffects();
		UE_LOG(LogTemp, Warning, TEXT("BLEED ENDS"), TickDamage)

	}
}