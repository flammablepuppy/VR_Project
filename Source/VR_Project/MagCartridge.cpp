// Copyright Aviator Games 2019

#include "MagCartridge.h"
#include "Components/StaticMeshComponent.h"
#include "WeaponMag.h"
#include "Components/SphereComponent.h"

AMagCartridge::AMagCartridge()
{

}
void AMagCartridge::BeginPlay()
{
	Super::BeginPlay();

}
void AMagCartridge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLoading)
	{
		MoveToMag();
	}
}
void AMagCartridge::SetTargetMag(AWeaponMag * NewTarget)
{
	TargetMagazine = NewTarget;
}

void AMagCartridge::LoadCartridge()
{
	if (!TargetMagazine) { return; }

	Drop();
	SetPickupEnabled(false);
	PickupMesh->SetSimulatePhysics(false);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->AttachToComponent(TargetMagazine->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	bLoading = true;

}

void AMagCartridge::MoveToMag()
{
	if (!TargetMagazine) { bLoading = false; return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector CurrentLocation = PickupMesh->GetComponentLocation();
	FVector TargetLocation = TargetMagazine->GetCartridgeLoadSphere()->GetComponentLocation();
	FVector DeltaLocation = TargetLocation - CurrentLocation;

	FVector NewLocation = CurrentLocation + DeltaLocation * DeltaTime / LoadSpeed;

	if (DeltaLocation.IsNearlyZero(0.1f))
	{
		LoadMag();
	}
	else
	{
		SetActorLocation(NewLocation);
	}

}

void AMagCartridge::LoadMag()
{
	TargetMagazine->ExpendCartridge(-1);
	Destroy();
}
