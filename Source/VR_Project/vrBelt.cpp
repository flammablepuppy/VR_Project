// Copyright Aviator Games 2019

#include "vrBelt.h"
#include "vrPlayer.h"
#include "Camera/CameraComponent.h"
#include "vrPickup.h"
#include "vrHolster.h"
#include "Components/CapsuleComponent.h"

UvrBelt::UvrBelt()
{
	PrimaryComponentTick.bCanEverTick = true;

}
void UvrBelt::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Make belt spawn a specified number of holsters.
	
	FindAllHolsters();

	OwningPlayer = Cast<AvrPlayer>(GetOwner());
	if (OwningPlayer)
	{
		TrackedHeadset = OwningPlayer->GetHeadsetCam();
	}

}
void UvrBelt::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Adjust the rotation of the belt to keep it in front of the camera
	if (TrackedHeadset)
	{
		SetWorldRotation(FRotator(/*TrackedHeadset->GetComponentRotation().Pitch / 2.f*/0.f, TrackedHeadset->GetComponentRotation().Yaw, 0.f));

		SetWorldLocation(FVector(
			//OwningPlayer->GetRootComponent()->GetComponentLocation().X,
			//OwningPlayer->GetRootComponent()->GetComponentLocation().Y,
			TrackedHeadset->GetComponentLocation().X,
			TrackedHeadset->GetComponentLocation().Y,
			TrackedHeadset->GetComponentLocation().Z - 60.f ));
	}
}

/** Resets and populates the array containing all the attached holsters */
void UvrBelt::FindAllHolsters()
{
	EquippedHolsters.Reset();

	OwningPlayer = Cast<AvrPlayer>(GetOwner());
	if (OwningPlayer)
	{
		TArray<AActor*> FoundActors;
		OwningPlayer->GetAllChildActors(FoundActors);
		for (AActor* FoundActor : FoundActors)
		{
			AvrHolster* FoundHolster = Cast<AvrHolster>(FoundActor);
			if (FoundHolster)
			{
				EquippedHolsters.AddUnique(FoundHolster);
			}
		}
	}
}

/**
*	Finds a vacant holster
*	@param PickupRequestingHolster, Finds a holster that is specifically for this item
*	@param OverrideProximityRequirement, When true item will snap to the holster even if outside it's sphere
*/
AvrHolster * UvrBelt::GetVacantHolster(AvrPickup * PickupRequestingHolster, bool OverrideProximityRequirement)
{
	for (AvrHolster* Holster : EquippedHolsters)
	{
		// Check if a CompatiblePickup is set, if requesting item is the correct class and the holster is empty
		if (Holster->GetCompatiblePickup() && PickupRequestingHolster->IsA(Holster->GetCompatiblePickup()) && !Holster->GetHolsteredItem())
		{
			return Holster;
		}
		// If there is no CompatiblePickup set, the holster is empty, and the holster doesn't require proximity
		else if (!Holster->GetCompatiblePickup() && !Holster->GetHolsteredItem() && !Holster->GetProximityAttachEnabled())
		{
			return Holster;
		}
		// If there is no CompatiblePickup set, the holster is empty, the holster does require proximity but the override is true
		else if (!Holster->GetCompatiblePickup() && !Holster->GetHolsteredItem() && Holster->GetProximityAttachEnabled() && OverrideProximityRequirement == true)
		{
			return Holster;
		}
	}

	return nullptr;
}

/** Populates provided array with all the items currently attached to any holster on the belt */
void UvrBelt::GetHolsteredItems(TArray<AvrPickup*>& Items)
{
	Items.Reset();

	for (AvrHolster* Holster : EquippedHolsters)
	{
		// Check if item is co-located with the holster, otherwise it must not be holstered there anymore
		if ((Holster->GetHolsteredItem()->GetActorLocation() - Holster->GetActorLocation()).Size() < 0.5f)
		{
			Items.AddUnique(Holster->GetHolsteredItem());
		}
	}
}

/** TODO: Make a pickup item that spawns a holster when you pick it up 
*	TODO: Figure out how to check if there's already a holster in the place it's being told to spawn
*/

void UvrBelt::SpawnHolster(FVector BeltPosition, TSubclassOf<AvrHolster> HolsterType, AvrHolster*& OutHolster, bool RequiresProximity)
{
	// Check if spawning the holster will exceed number allowed
	if (EquippedHolsters.Num() < MaxHolsters)
	{
		// Spawn and attach the holster
		AvrHolster* SpawnedHolster = GetWorld()->SpawnActor<AvrHolster>(HolsterType, BeltPosition, GetComponentRotation());
		SpawnedHolster->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		SpawnedHolster->SetActorRelativeRotation(FRotator(0.f, 0.f, 0.f));

		// Add it to the array so OnDrop can be subbed on it automatically
		EquippedHolsters.AddUnique(SpawnedHolster);

		// Optional: Make is proximity dependent if 
		if (RequiresProximity)
		{
			SpawnedHolster->SetProximityEnabled(true);
		}

		OutHolster = SpawnedHolster;
	}
}

void UvrBelt::SetMaxHolsters(int32 NewMax)
{
	MaxHolsters = NewMax;
}
