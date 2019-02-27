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
	if (Value > 0.3f && !bTriggerPulled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trigger pull recognized."))

		bTriggerPulled = true;
		DischargeRound();
	}
	else if (Value < 0.3f)
	{
		bTriggerPulled = false;
	}
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
	UE_LOG(LogTemp, Warning, TEXT("DischargeRound called."))

	if (!bSlideBack && bRoundChambered)
	{
		UE_LOG(LogTemp, Warning, TEXT("Conditions met to spawn projectile and play FX."))

		GetWorld()->SpawnActor<AvrProjectile>(PistolMesh->GetSocketLocation("Muzzle"), PistolMesh->GetSocketRotation("Muzzle"));
		PlayFireFX(); // Has VFX and slide snap back montage
		bSlideBack = true;
		bRoundChambered = false;

		AttemptCharge();
	}
	else
	{
		// Play SFX of hammer snapping, ie. failure to fire
	}
}

void ASigPistol::AttemptCharge()
{
	if (bSlideBack && LoadedMagazine)
	{
		//if (LoadedMagazine->RemainingRounds > 0)
		//{
		//	// -1 ammo
		//	// play slide snap forward
		//	// bRoundChambered = true;
		//	// bSlideForward = true;
		//}

	}
}
