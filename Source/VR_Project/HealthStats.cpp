// Copyright Aviator Games 2019

#include "HealthStats.h"
#include "GameFramework/Actor.h"
#include "vrPlayer.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "vrPickup.h"

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
		UE_LOG(LogTemp, Warning, TEXT("HealthStats attached and working for: %s"), *GetOwner()->GetName())
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
			PlayerDeath();
			bOwnerIsDead = true;
		}
	}
}
void UHealthStats::PlayerDeath()
{
	AvrPlayer* OwningPlayer = Cast<AvrPlayer>(GetOwner());
	if (OwningPlayer)
	{
		//OwningPlayer->DisableInput(Cast<APlayerController>(OwningPlayer->GetController()));
		if (OwningPlayer->GetLeftHeldObject()) { OwningPlayer->GetLeftHeldObject()->Drop();	}
		if (OwningPlayer->GetRightHeldObject()) { OwningPlayer->GetRightHeldObject()->Drop(); }
		OwningPlayer->GetMovementComponent()->StopActiveMovement();
		bShowDeathMessage = true;
	}
}
