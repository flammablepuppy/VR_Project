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
	if (OverlappingPickup && OverlappingPickup->GetOwningMC())
	{
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

	TSet<AActor*> ActorSet;
	AvrPickup* ClosestPickup = nullptr;
	HolsterSphere->GetOverlappingActors(ActorSet);
	for (AActor* FoundActor : ActorSet)
	{
		AvrPickup* FoundPickup = Cast<AvrPickup>(FoundActor);
		if (FoundPickup)
		{
			FoundPickup->OnDrop.RemoveAll(this);

			if (!ClosestPickup)
			{
				ClosestPickup = FoundPickup;
			}
			else
			{
				float NewDistance = (HolsterSphere->GetComponentLocation() - FoundPickup->GetActorLocation()).Size();
				float OldDistance = (HolsterSphere->GetComponentLocation() - ClosestPickup->GetActorLocation()).Size();
				if (NewDistance < OldDistance)
				{
					ClosestPickup = FoundPickup;
				}
			}
		}
	}

	if (ClosestPickup) 
	{ 
		ClosestPickup->OnSnappedOn.AddUniqueDynamic(this, &AvrHolster::EnableHolsteredItem);
		ClosestPickup->SnapInitiate(HolsterSphere); 
	}
}

void AvrHolster::EnableHolsteredItem(AvrPickup* PickupToEnable)
{
	PickupToEnable->SetPickupEnabled(true);
	PickupToEnable->SetActorEnableCollision(true);
	PickupToEnable->OnSnappedOn.RemoveAll(this);
}

