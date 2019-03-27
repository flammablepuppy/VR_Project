// Copyright Aviator Games 2019

#include "vrBelt.h"
#include "vrPlayer.h"
#include "Camera/CameraComponent.h"

UvrBelt::UvrBelt()
{
	PrimaryComponentTick.bCanEverTick = true;

}
void UvrBelt::BeginPlay()
{
	Super::BeginPlay();

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
			OwningPlayer->GetRootComponent()->GetComponentLocation().X,
			OwningPlayer->GetRootComponent()->GetComponentLocation().Y,
			TrackedHeadset->GetComponentLocation().Z - 50.f));
	}
}

