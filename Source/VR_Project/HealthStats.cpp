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

UHealthStats::UHealthStats()
{
	PrimaryComponentTick.bCanEverTick = false;

}
void UHealthStats::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaximumHealth;
	OwningPlayer = Cast<AvrPlayer>(GetOwner());
	
	if (OwningPlayer)
	{
		OwningPlayer->OnTakeAnyDamage.AddDynamic(this, &UHealthStats::OwnerTakesDamage);
	}
	else
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthStats::OwnerTakesDamage);
		}
	}
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

	YardSale();

}

void UHealthStats::SetIsDead(bool NewState)
{
	bOwnerIsDead = NewState;
}

void UHealthStats::Respawn()
{
	OwningPlayer->EnableInput(Cast<APlayerController>(OwningPlayer->GetController()));
	OwnerTakesDamage(OwningPlayer, -MaximumHealth, nullptr, nullptr, nullptr);
	bOwnerIsDead = false;
	OnRespawn.Broadcast(GetOwner());
}

/** Owning Player drops all their stuff */
void UHealthStats::YardSale(float DroppedItemsLifespan)
{
	if (DroppedItemsLifespan > 0.f)
	{
		TArray<AvrPickup*> PlayerInventory;
		MemorizePlayerItems(PlayerInventory);
		for (AvrPickup* Item : PlayerInventory)
		{
			Item->SetLifeSpan(DroppedItemsLifespan);
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

