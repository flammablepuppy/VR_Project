// Copyright Aviator Games 2019

#include "WeaponMag.h"
#include "Components/SphereComponent.h"
#include "vrProjectile.h"
#include "MagCartridge.h"
#include "Components/StaticMeshComponent.h"
#include "SigPistol.h"
#include "MotionControllerComponent.h"
#include "vrPlayer.h"
#include "vrHolster.h"
#include "vrPlayer.h"
#include "vrBelt.h"
#include "TimerManager.h"

AWeaponMag::AWeaponMag()
{
	CartridgeLoadSphere = CreateDefaultSubobject<USphereComponent>("Cartridge Load Sphere");
	CartridgeLoadSphere->SetupAttachment(RootComponent);

}
void AWeaponMag::BeginPlay()
{
	Super::BeginPlay();

	CartridgeLoadSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponMag::LoadCompatibleCartridge);
	CartridgeLoadSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponMag::UnPrimeCartridge);

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
			Cartridge->Drop();
			Cartridge->SetTargetMag(this);
			Cartridge->SnapInitiate(CartridgeLoadSphere);
			Cartridge->SetPickupEnabled(false);
		}
		else if (Cartridge)
		{
			Cartridge->SetTargetMag(this);
		}
	}
}
void AWeaponMag::UnPrimeCartridge(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AMagCartridge* Cartridge = Cast<AMagCartridge>(OtherActor);
	if (Cartridge)
	{
		Cartridge->SetTargetMag(nullptr);
	}
}

void AWeaponMag::SetCapacity(int32 NewCurrentCapacity)
{
	CurrentCapacity = NewCurrentCapacity;
}
void AWeaponMag::ExpendCartridge(int32 RoundsExpended)
{
	CurrentCapacity = FMath::Clamp(CurrentCapacity -= RoundsExpended, 0, MaxCapacity);
}

void AWeaponMag::SetLoading(bool NewState)
{
	bLoading = NewState;
}
void AWeaponMag::SnapOn()
{
	Super::SnapOn();
	
	AvrHolster* OwnedByHolster = Cast<AvrHolster>(SnapTarget->GetOwner());
	if (!OwningMC && !OwnedByHolster) { PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); }

}
void AWeaponMag::Drop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PickupMesh->SetSimulatePhysics(true);

	if (bReadyToUse && OwningMC) { OwningMC->SetShowDeviceModel(false); bReadyToUse = false; }
	else { bReadyToUse = false; }

	if (OwningMC) { OwningMC = nullptr; }
	if (OwningPlayer)
	{
		AvrHolster* VacantHolster = Cast<AvrHolster>(OwningPlayer->GetUtilityBelt()->GetVacantHolster(this));
		if (VacantHolster && (GetVelocity() - OwningPlayer->GetVelocity()).Size() < NoHolsterSpeed && bSeeksHolster)
		{
			// This is the crucial part of the override that prevents a magazine being loaded into a weapon from 
			// snapping into a vacant holster instead of the weapon
			if (bLoading)
			{
				SetLoading(false);
			}
			else
			{
				OnDrop.Clear();
				OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
			}
		}
		OwningPlayer = nullptr;
	}

	SnapTarget = nullptr;
	SnapSocket = NAME_None;
	bPickupEnabled = true;

	OnDrop.Broadcast(this);
	OnDrop.Clear();

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

}

/**
*	Called by the weapon this magazine was ejected from
*	After a certain amount of time, search for a vacanat holster to store the magazine and send it there
*/
void AWeaponMag::SetMagSearchForHolster(AvrPlayer* PlayerToHolster)
{
	TargetPlayerForHolstering = PlayerToHolster;
	FTimerHandle HolsterSnapDelay_Timer;
	GetWorldTimerManager().SetTimer(HolsterSnapDelay_Timer, this, &AWeaponMag::SnapToVacantHolster, 0.65f);
}

/**
*	Snaps magazine to a holster 
*	Checks if the magazine is already held, that there is a vacant holster that can hold it and that it's not too far away
*/
void AWeaponMag::SnapToVacantHolster()
{
	if (SnapTarget)
	{
		return; 
	}

	AvrHolster* VacantHolster = Cast<AvrHolster>(TargetPlayerForHolstering->GetUtilityBelt()->GetVacantHolster(this));
	if (VacantHolster && 
		(GetActorLocation() - VacantHolster->GetActorLocation()).Size() < 300.f)
	{
		OnDrop.AddUniqueDynamic(VacantHolster, &AvrHolster::CatchDroppedPickup);
		Drop();
	}
	else
	{
		OnDrop.Clear();
	}

	TargetPlayerForHolstering = nullptr;
}
