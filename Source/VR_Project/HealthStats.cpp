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
//void UHealthStats::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//}

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
				vrPlayerOwner->DisableInput(Cast<APlayerController>(vrPlayerOwner->GetController()));
				if (vrPlayerOwner->GetLeftHeldObject()) { vrPlayerOwner->GetLeftHeldObject()->Drop(); }
				if (vrPlayerOwner->GetRightHeldObject()) { vrPlayerOwner->GetRightHeldObject()->Drop(); }
				vrPlayerOwner->GetMovementComponent()->StopActiveMovement();

				bOwnerIsDead = true;

				FTimerHandle ReLoadLevel_Timer;
				GetWorld()->GetTimerManager().SetTimer(ReLoadLevel_Timer, this, &UHealthStats::PlayerDeath, 3.f);

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
void UHealthStats::PlayerDeath()
{
	FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName));

}
