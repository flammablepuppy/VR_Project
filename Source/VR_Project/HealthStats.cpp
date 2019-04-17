// Copyright Aviator Games 2019

#include "HealthStats.h"
#include "GameFramework/Actor.h"
#include "vrPlayer.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "vrPickup.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UHealthStats::UHealthStats()
{
	PrimaryComponentTick.bCanEverTick = false;

}
void UHealthStats::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaximumHealth;

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthStats::OwnerTakesDamage);
	}
}

void UHealthStats::HandleDeath()
{
	AvrPlayer* Owner = Cast<AvrPlayer>(GetOwner());
	Owner->DisableInput(Cast<APlayerController>(Owner->GetController()));
	if (Owner->GetLeftHeldObject()) { Owner->GetLeftHeldObject()->Drop(); }
	if (Owner->GetRightHeldObject()) { Owner->GetRightHeldObject()->Drop(); }
	Owner->GetMovementComponent()->StopActiveMovement();

}

void UHealthStats::OwnerTakesDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage > 0.f || Damage < 0.f )
	{
		DamageTaken.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);

		if (CurrentHealth <= 0.f && !bOwnerIsDead)
		{
			bOwnerIsDead = true;

			AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
			if (vrPlayerOwner)
			{
				HandleDeath();
			}
			if (OnDeath.IsBound())
			{
				OnDeath.Broadcast(GetOwner());
			}
			else
			{
				GetOwner()->Destroy();
			}
		}
	}
}

/** Sets CheckpointLocation variable for use in respawning */
void UHealthStats::SetCheckpointLocation(FVector NewLocation)
{
	CheckpointLocation = NewLocation;
}

void UHealthStats::SetIsDead(bool NewState)
{
	bOwnerIsDead = NewState;
}

void UHealthStats::AdjustCurrency(float CurrencyAdjustment)
{
	Currency += CurrencyAdjustment;
}