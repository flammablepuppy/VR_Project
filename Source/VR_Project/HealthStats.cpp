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

void UHealthStats::Die()
{
	AvrPlayer* Owner = Cast<AvrPlayer>(GetOwner());
	Owner->DisableInput(Cast<APlayerController>(Owner->GetController()));
	if (Owner->GetLeftHeldObject()) { Owner->GetLeftHeldObject()->Drop(); }
	if (Owner->GetRightHeldObject()) { Owner->GetRightHeldObject()->Drop(); }
	Owner->GetMovementComponent()->StopActiveMovement();

}

void UHealthStats::OwnerTakesDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage > 0.f || Damage < 0.f)
	{
		DamageTaken.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);

		if (CurrentHealth <= 0.f && !bOwnerIsDead)
		{
			bOwnerIsDead = true;

			AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
			if (vrPlayerOwner)
			{
				Die();
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

void UHealthStats::SetIsDead(bool NewState)
{
	bOwnerIsDead = NewState;
}

void UHealthStats::Respawn()
{
	AvrPlayer* Owner = Cast<AvrPlayer>(GetOwner());
	Owner->EnableInput(Cast<APlayerController>(Owner->GetController()));
	OwnerTakesDamage(GetOwner(), -500.f, nullptr, nullptr, nullptr);
	bOwnerIsDead = false;

	OnRespawn.Broadcast(GetOwner());
}

void UHealthStats::AdjustCurrency(float CurrencyAdjustment)
{
	Currency += CurrencyAdjustment;
}