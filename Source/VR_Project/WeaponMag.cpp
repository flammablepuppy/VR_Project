// Copyright Aviator Games 2019

#include "WeaponMag.h"
#include "Components/SphereComponent.h"
#include "vrProjectile.h"
#include "MagCartridge.h"
#include "Components/StaticMeshComponent.h"

AWeaponMag::AWeaponMag()
{
	CartridgeLoadSphere = CreateDefaultSubobject<USphereComponent>("Cartridge Load Sphere");
	CartridgeLoadSphere->SetupAttachment(RootComponent);

}

void AWeaponMag::BeginPlay()
{
	Super::BeginPlay();

	CartridgeLoadSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponMag::LoadCompatibleCartridge);

	// Initialize CurrentCapacity to MaxCapacity
	if (CurrentCapacity == -1) { CurrentCapacity = MaxCapacity;	}

}

void AWeaponMag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponMag::LoadCompatibleCartridge(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (CompatibleCartidge &&					// Check for nullptr, make sure it's set
		OtherActor->IsA(CompatibleCartidge) &&	// Make sure it's the correct cartridge type
		CurrentCapacity < MaxCapacity)			// Make sure the mag isn't already full
	{
		AMagCartridge* Cartridge = Cast<AMagCartridge>(OtherActor);
		if (Cartridge && Cartridge->GetOwningMC() && OwningMC)
		{
			Cartridge->SetTargetMag(this);
			Cartridge->LoadCartridge();
		}
	}
}

void AWeaponMag::SetCapacity(int32 NewCurrentCapacity)
{
	CurrentCapacity = NewCurrentCapacity;
}

void AWeaponMag::ExpendCartridge(int32 RoundsExpended)
{
	FMath::Clamp(CurrentCapacity -= RoundsExpended, 0, MaxCapacity);
}

void AWeaponMag::SnapOn()
{
	Super::SnapOn();

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponMag::Drop()
{
	Super::Drop();

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
