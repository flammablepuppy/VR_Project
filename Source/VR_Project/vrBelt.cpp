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

	if (TrackedHeadset)
	{
		SetWorldRotation(FRotator(TrackedHeadset->GetComponentRotation().Pitch / 2.f, TrackedHeadset->GetComponentRotation().Yaw, 0.f));

		SetWorldLocation(FVector(
			//OwningPlayer->GetRootComponent()->GetComponentLocation().X,
			//OwningPlayer->GetRootComponent()->GetComponentLocation().Y,
			TrackedHeadset->GetComponentLocation().X,
			TrackedHeadset->GetComponentLocation().Y,
			TrackedHeadset->GetComponentLocation().Z - 60.f ));
	}
}

void UvrBelt::FindAllHolsters()
{
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
AvrHolster * UvrBelt::GetVacantHolster(AvrPickup * PickupRequestingHolster, bool OverrideProximityRequirement)
{
	for (AvrHolster* Holster : EquippedHolsters)
	{
		// Check if a CompatiblePickup is set, if requesting item is the correct class and the holster is empty
		if (Holster->GetCompatiblePickup() && PickupRequestingHolster->IsA(Holster->GetCompatiblePickup()) && !Holster->GetHolsteredItem())
		{
			return Holster;
		}
		// If there is no CompatiblePickup set, it just has to be empty
		else if (!Holster->GetCompatiblePickup() && !Holster->GetHolsteredItem() && !Holster->GetProximityAttachEnabled())
		{
			return Holster;
		}
		else if (!Holster->GetCompatiblePickup() && !Holster->GetHolsteredItem() && OverrideProximityRequirement)
		{
			return Holster;
		}
	}

	return nullptr;
}

void UvrBelt::GetHolsteredItems(TArray<AvrPickup*>& Items)
{
	Items.Reset();

	for (AvrHolster* Holster : EquippedHolsters)
	{
		Items.AddUnique(Holster->GetHolsteredItem());
	}
}
