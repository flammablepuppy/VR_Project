// Copyright Aviator Games 2019


#include "StatsHandler.h"
#include "HealthStats.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "vrPlayer.h"
#include "TimerManager.h"

/**
*
*/

UStatsHandler::UStatsHandler()
{

}

void UStatsHandler::BeginPlay()
{
	Super::BeginPlay();
	
	DetermineBaseStats();
}

/**
*
*/

void UStatsHandler::DetermineBaseStats()
{
	AvrPlayer* vrPlayerOwner = Cast<AvrPlayer>(GetOwner());
	UHealthStats* Health = GetOwner()->FindComponentByClass<UHealthStats>();
	UCharacterMovementComponent* Movement = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();

	if (Health && Movement)
	{
		Stats.Health = Health->GetMaxHealth();
		Stats.WalkSpeed = Movement->MaxWalkSpeed;
		if (vrPlayerOwner) Stats.SprintSpeed = vrPlayerOwner->GetSprintSpeed();
	}
}

void UStatsHandler::ApplySlow(float Power, float Duration)
{
	FTimerHandle Slow_Timer;
	FTimerDelegate Slow_Delegate;
	Slow_Delegate.BindUFunction(this, "UndoSlow", Power);

	GetOwner()->FindComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed *= 1.f - Power;
	GetWorld()->GetTimerManager().SetTimer(Slow_Timer, Slow_Delegate, Duration, false);

}

void UStatsHandler::UndoSlow(float Power)
{
	GetOwner()->FindComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed /= 1.f - Power;

}
