// Copyright Aviator Games 2019

#include "WeaponMag.h"
#include "vrProjectile.h"

AWeaponMag::AWeaponMag()
{

}

void AWeaponMag::BeginPlay()
{
	Super::BeginPlay();

	// Initialize CurrentCapacity to MaxCapacity
	if (CurrentCapacity == -1) { CurrentCapacity = MaxCapacity;	}

}

void AWeaponMag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponMag::SetCapacity(int32 NewCurrentCapacity)
{
	CurrentCapacity = NewCurrentCapacity;
}

void AWeaponMag::ExpendCartridge(int32 RoundsExpended)
{
	if (CurrentCapacity > 0)
	{
		CurrentCapacity -= RoundsExpended;
	}
}
