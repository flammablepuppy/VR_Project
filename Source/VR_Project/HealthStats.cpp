// Copyright Aviator Games 2019

#include "HealthStats.h"
#include "GameFramework/Actor.h"
#include "vrPlayer.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	if (bOwnerIsDead) { return; }

	if (Damage > 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);
		DamageFloatingNumber(Damage);
		UE_LOG(LogTemp, Log, TEXT("Current health: %f after DAMAGE"), CurrentHealth)

		if (CurrentHealth <= 0.f)
		{
			PlayerDeath();
			bOwnerIsDead = true;
		}
	}
	if (Damage < 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);
		UE_LOG(LogTemp, Log, TEXT("Current health: %f after HEALING"), CurrentHealth)

	}
}
void UHealthStats::PlayerDeath()
{
	AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwner());
	if (OwningPlayer)
	{
		OwningPlayer->GetLeftMC()->SetSimulatePhysics(true);
		OwningPlayer->GetRightMC()->SetSimulatePhysics(true);
		//OwningPlayer->DisableInput(Cast<APlayerController>(OwningPlayer->GetController()));
		OwningPlayer->GetMovementComponent()->StopActiveMovement();
		bShowDeathMessage = true;
	}
}
