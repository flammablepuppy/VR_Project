// Copyright Aviator Games 2019

#include "vrHolster.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "vrPickup.h"
#include "WeaponMag.h"

AvrHolster::AvrHolster()
{
	PrimaryActorTick.bCanEverTick = true;

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
void AvrHolster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bScanRunning && bProximityAttachEnabled)
	{
		ScanForPickupsToCatch();
	}
}

/**
*
*/

/** Starts scan */
void AvrHolster::SubscribeCatch(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	AvrPickup* OverlappingPickup = Cast<AvrPickup>(OtherActor);
	if (OverlappingPickup && OverlappingPickup->GetOwningMC() && !HolsteredItem && bProximityAttachEnabled)
	{
		// If compatibility filter is set, check that this item is compatible
		if (CompatiblePickup && OverlappingPickup->IsA(CompatiblePickup))
		{
			bScanRunning = true;

		}
		// If no compatbility filter is set, always work
		else if (!CompatiblePickup)
		{
			bScanRunning = true;

		}
	}
}
/** Stops scan */
void AvrHolster::UnsubCatch(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AvrPickup* OverlappingPickup = Cast<AvrPickup>(OtherActor);
	if (OverlappingPickup && OverlappingPickup->GetOwningMC() && bProximityAttachEnabled)
	{
		OverlappingPickup->OnDrop.Clear();
	}
}
/** Subscribes the closest vrPickup in the sphere to be caught when dropped */
void AvrHolster::ScanForPickupsToCatch()
{
	// Will be pointed to closest compatible pickup to be subbed to catch
	AvrPickup* SubscriptionTarget = nullptr;

	TSet<AActor*> FoundActors;
	HolsterSphere->GetOverlappingActors(FoundActors);
	for (AActor* FoundActor : FoundActors)
	{
		AvrPickup* CompatibleFoundPickup = Cast<AvrPickup>(FoundActor);
		if (CompatibleFoundPickup)
		{
			if (!SubscriptionTarget) { SubscriptionTarget = CompatibleFoundPickup; }

			float SubDistance = (HolsterSphere->GetComponentLocation() - SubscriptionTarget->GetActorLocation()).Size();
			float FoundDistance = (HolsterSphere->GetComponentLocation() - CompatibleFoundPickup->GetActorLocation()).Size();
			if (FoundDistance < SubDistance)
			{
				SubscriptionTarget = CompatibleFoundPickup;
			}

		}
	}

	// Stop the scanning if the holster is occupied
	if (HolsteredItem) { SubscriptionTarget = nullptr; }
	// Stop scanning if there are no vrPickups in the volume
	if (!SubscriptionTarget) { bScanRunning = false; }

	if (SubscriptionTarget)
	{
		SubscriptionTarget->OnDrop.Clear();
		SubscriptionTarget->OnDrop.AddUniqueDynamic(this, &AvrHolster::CatchDroppedPickup);
	}
}

/** THIS IS HOW YOU ATTACH THINGS TO THE HOLSTER <------!!!
*	Called to holster an item in this holster
*	To use, subscribe a vrPickup OnDrop to this function ie "vrPickup->OnDrop.AddUniqueDynamic(vrHolster, &AvrHolster::CatchDroppedPickup);"
*	CANNOT be called directly by a pickup, will not function correctly
*/
void AvrHolster::CatchDroppedPickup(AvrPickup* DroppedPickup)
{
	if (HolsteredItem) 
	{ 
		return; 
	}

	HolsteredItem = DroppedPickup;
	HolsteredItem->OnSnappedOn.AddUniqueDynamic(this, &AvrHolster::EnableHolsteredItem);
	HolsteredItem->SnapInitiate(HolsterSphere);

	HolsterMesh->SetVisibility(false);
}

/** Called when a caught pickup snaps onto the holster, allowing it to be grabbed out of the holster */
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

/** Called when holstered pickup is retrieved from the holster, freeing up the holster and allowing it to scan again */
void AvrHolster::ClearHolsteredItem(AvrPickup* DroppedPickup)
{
	if (HolsteredItem)
	{
		HolsteredItem->OnGrabbed.Clear();
		HolsteredItem = nullptr;
		bScanRunning = true; // Allows dropped item to immedietly be dropped back in holster without having to EndOverlap and then Overlap

		HolsterMesh->SetVisibility(true);
	}
}

void AvrHolster::SetCompatiblePickup(TSubclassOf<AvrPickup> NewCompatiblePickup)
{
	CompatiblePickup = NewCompatiblePickup;
}

void AvrHolster::SetProximityEnabled(bool NewState)
{
	bProximityAttachEnabled = NewState;
}

