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

void UHealthStats::OwnerTakesDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage > 0.f || Damage < 0.f )
	{
		DamageTaken.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);

		if (CurrentHealth <= 0.f)
		{
			AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
			if (vrPlayerOwner)
			{
				PlayerDeath();
			}
			else
			{
				if (OnDeath.IsBound())
				{
					OnDeath.Broadcast();
				}
				else
				{
					GetOwner()->Destroy();
				}
			}
		}
	}
}

/** Sets CheckpointLocation variable for use in respawning */
void UHealthStats::SetCheckpointLocation(FVector NewLocation)
{
	CheckpointLocation = NewLocation;
}

void UHealthStats::AdjustCurrency(float CurrencyAdjustment)
{
	Currency += CurrencyAdjustment;
}

/** Handles player death and calls for respawn */
void UHealthStats::PlayerDeath()
{
	AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
	if (vrPlayerOwner)
	{
		vrPlayerOwner->DisableInput(Cast<APlayerController>(vrPlayerOwner->GetController()));
		if (vrPlayerOwner->GetLeftHeldObject()) { vrPlayerOwner->GetLeftHeldObject()->Drop(); }
		if (vrPlayerOwner->GetRightHeldObject()) { vrPlayerOwner->GetRightHeldObject()->Drop(); }
		vrPlayerOwner->GetMovementComponent()->StopActiveMovement();
	}

	bOwnerIsDead = true;

	FTimerHandle ReLoadLevel_Timer;
	GetWorld()->GetTimerManager().SetTimer(ReLoadLevel_Timer, this, &UHealthStats::RespawnPlayer, 2.f);

	if (OnDeath.IsBound())
	{
		OnDeath.Broadcast();
	}

}

/**
*	If a checkpoint has been reached teleports to checkpoint
*	If not reloads level
*/
void UHealthStats::RespawnPlayer()
{
	if (CheckpointLocation != FVector::ZeroVector)
	{
		AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
		if (vrPlayerOwner)
		{
			// Reverse death effects
			vrPlayerOwner->EnableInput(Cast<APlayerController>(vrPlayerOwner->GetController()));
			CurrentHealth = MaximumHealth;
			bOwnerIsDead = false;

			// Move player to checkpoint and zero their velocity
			GetOwner()->SetActorLocation(CheckpointLocation);
			vrPlayerOwner->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			vrPlayerOwner->GetCharacterMovement()->UpdateComponentVelocity();

			if (OnRespawn.IsBound())
			{
				OnRespawn.Broadcast();
			}
		}
	}
	else
	{
		FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
		UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName));
	}
}
