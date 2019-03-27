// Copyright Aviator Games 2019

#include "vrHolster.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPickup.h"
#include "WeaponMag.h"

AvrHolster::AvrHolster()
{
	//PrimaryActorTick.bCanEverTick = true;

	HolsterMesh = CreateDefaultSubobject<UStaticMeshComponent>("Holster Mesh");
	HolsterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = HolsterMesh;

	HolsterSphere = CreateDefaultSubobject<USphereComponent>("Holster Sphere");
	HolsterSphere->SetupAttachment(RootComponent);

}
void AvrHolster::BeginPlay()
{
	Super::BeginPlay();

	HolsterSphere->OnComponentBeginOverlap.AddDynamic(this, &AvrHolster::SubscribeCatch);
	HolsterSphere->OnComponentEndOverlap.AddDynamic(this, &AvrHolster::UnsubCatch);
	
}
//void AvrHolster::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AvrHolster::SubscribeCatch(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPickup* OverlappingPickup = Cast<AvrPickup>(OtherActor);
	if (OverlappingPickup && OverlappingPickup->GetOwningMC() && !HolsteredItem)
	{
		OverlappingPickup->OnDrop.Clear();
		OverlappingPickup->OnDrop.AddUniqueDynamic(this, &AvrHolster::CatchDroppedPickup);
	}
}
void AvrHolster::UnsubCatch(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AvrPickup* OverlappingPickup = Cast<AvrPickup>(OtherActor);
	if (OverlappingPickup && OverlappingPickup->GetOwningMC())
	{
		OverlappingPickup->OnDrop.RemoveAll(this);
	}
}

void AvrHolster::CatchDroppedPickup(AvrPickup* DroppedPickup)
{
	if (HolsteredItem)
	{
		ClearHolsteredItem(DroppedPickup);
	}

	HolsteredItem = DroppedPickup;
	HolsteredItem->OnSnappedOn.AddUniqueDynamic(this, &AvrHolster::EnableHolsteredItem);
	HolsteredItem->SnapInitiate(HolsterSphere);

	HolsterMesh->SetVisibility(false);
}

void AvrHolster::EnableHolsteredItem(AvrPickup* PickupToEnable)
{
	if (PickupToEnable == HolsteredItem)
	{
		HolsteredItem->SetPickupEnabled(true);
		HolsteredItem->SetActorEnableCollision(true);
		HolsteredItem->OnSnappedOn.Clear();

		HolsteredItem->OnGrabbed.AddUniqueDynamic(this, &AvrHolster::ClearHolsteredItem);
	}
}

void AvrHolster::ClearHolsteredItem(AvrPickup* DroppedPickup)
{
	if (HolsteredItem)
	{
		HolsteredItem->OnGrabbed.Clear();
		HolsteredItem = nullptr;

		HolsterMesh->SetVisibility(true);
	}
}

