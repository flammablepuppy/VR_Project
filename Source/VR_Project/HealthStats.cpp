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
	OwningPlayer->EnableInput(Cast<APlayerController>(OwningPlayer->GetController()));
	SetCurrentHealth(MaximumHealth);
	bOwnerIsDead = false;

	FTimerHandle EnableDamage_Handle;
	FTimerDelegate EnableDamageDelegate;
	EnableDamageDelegate.BindUFunction(OwningPlayer, FName("SetImpactDamageEnabled"), true);
	GetWorld()->GetTimerManager().SetTimer(EnableDamage_Handle, EnableDamageDelegate, 0.5f, false);

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

void UHealthStats::SetCurrentHealth(float NewHealth)
{
	CurrentHealth = NewHealth;
}

