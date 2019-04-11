// Copyright Aviator Games 2019

#include "MagCartridge.h"
#include "Components/StaticMeshComponent.h"
#include "WeaponMag.h"
#include "Components/SphereComponent.h"
#include "vrProjectile.h"

AMagCartridge::AMagCartridge()
{
	bSeeksHolster = false;
}
void AMagCartridge::BeginPlay()
{
	Super::BeginPlay();

}
void AMagCartridge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagCartridge::SnapInitiate(USceneComponent * NewParentComponent, FName SocketName)
{
	if (TargetMagazine && TargetMagazine->GetOwningMC())
	{
		Super::SnapInitiate(TargetMagazine->GetCartridgeLoadSphere(), SocketName);

	}
	else
	{
		Super::SnapInitiate(NewParentComponent, SocketName);
	}
}

void AMagCartridge::SnapOn()
{
	Super::SnapOn();

	if (TargetMagazine && TargetMagazine->GetOwningMC()) { LoadMag(); }
}

void AMagCartridge::SetTargetMag(AWeaponMag * NewTarget)
{
	TargetMagazine = NewTarget;
}

void AMagCartridge::LoadMag()
{
	if (TargetMagazine->GetCurrentCapacity() < TargetMagazine->GetMaxCapacity())
	{
		TargetMagazine->ExpendCartridge(-1);
		BP_PlayCartridgeLoad();

		Destroy();
	}
	else
	{
		Drop();
		TargetMagazine = nullptr;
	}
}
