// Copyright Aviator Games 2019

#include "HealthStats.h"

UHealthStats::UHealthStats()
{
	PrimaryComponentTick.bCanEverTick = true;

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


void UHealthStats::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthStats::OwnerTakesDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage > 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);
		UE_LOG(LogTemp, Log, TEXT("Current health: %f after DAMAGE"), CurrentHealth)

	}
	if (Damage < 0.f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaximumHealth);
		UE_LOG(LogTemp, Log, TEXT("Current health: %f after HEALING"), CurrentHealth)

	}
}

