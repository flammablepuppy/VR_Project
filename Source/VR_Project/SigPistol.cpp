// Copyright Aviator Games 2019

#include "SigPistol.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "vrProjectile.h"

ASigPistol::ASigPistol()
{
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Pistol Skeletal Mesh");
	PistolMesh->SetCollisionObjectType(ECC_PhysicsBody);
	PistolMesh->SetupAttachment(RootComponent);
	
}

// Input Functions
void ASigPistol::Drop()
{
	Super::Drop();

}
void ASigPistol::TriggerPulled(float Value)
{
	DischargeRound();
}
void ASigPistol::TopPushed()
{
}
void ASigPistol::TopReleased()
{
}
void ASigPistol::BottomPushed()
{
}
void ASigPistol::BottomReleased()
{
}

// Functionality
void ASigPistol::DischargeRound()
{
	if (!bSlideBack && bRoundChambered)
	{
		GetWorld()->SpawnActor<AvrProjectile>(PistolMesh->GetSocketLocation("Muzzle"), PistolMesh->GetSocketRotation("Muzzle"));
		// Play slide snap back animation
		bSlideBack = true;
		bRoundChambered = false;

		// If magazine still has ammo in it
		AttemptCharge();
	}
	else
	{
		// Play SFX of hammer snapping, ie. failure to fire
	}
}

void ASigPistol::AttemptCharge()
{
	if (bSlideBack)
	{
		// -1 ammo from mag then

	}
}
