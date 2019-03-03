// Copyright Aviator Games 2019

#include "SigPistol.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "vrProjectile.h"
#include "Components/SphereComponent.h"
#include "WeaponMag.h"

ASigPistol::ASigPistol()
{
	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Pistol Skeletal Mesh");
	PistolMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PistolMesh->SetupAttachment(RootComponent);

	MagazineLoadSphere = CreateDefaultSubobject<USphereComponent>("Magazine Load Sphere");
	MagazineLoadSphere->SetupAttachment(RootComponent); // Would be nice to automatically set this to a predetermined socket by
	
}
void ASigPistol::BeginPlay()
{
	Super::BeginPlay();

	MagazineLoadSphere->OnComponentBeginOverlap.AddDynamic(this, &ASigPistol::MagOverlap);

	if (bSpawnsLoaded)
	{
		LoadedMagazine = GetWorld()->SpawnActor<AWeaponMag>(StarterMagazine);
		AttachMag();
		ChamberedRound = LoadedMagazine->GetLoadedAmmunition();

		if (StarterCapacity != -1)
		{
			LoadedMagazine->SetCapacity(StarterCapacity);
		}
	}

}
void ASigPistol::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ASigPistol::MagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (LoadedMagazine) { return; }

	AWeaponMag* Magazine = Cast<AWeaponMag>(OtherActor);
	if (Magazine)
	{
		Magazine->Drop();
		LoadedMagazine = Magazine;
		AttachMag();
	}

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
	if (bSlideBack)
	{
		AttemptCharge();
		if (!ChamberedRound)
		{
			PlaySlideForward();
			bSlideBack = false;
		}
	}
	else if (!bSlideBack)
	{
		PlaySlideBack();
		bSlideBack = true;
	}
}
void ASigPistol::TopReleased()
{
}
void ASigPistol::BottomPushed()
{
	if (LoadedMagazine)
	{
		DropMag();
	}

}
void ASigPistol::BottomReleased()
{
}

// Functionality
void ASigPistol::DischargeRound()
{
	if (!bSlideBack && ChamberedRound)
	{
		GetWorld()->SpawnActor<AvrProjectile>(ChamberedRound, PistolMesh->GetSocketLocation("Muzzle"), PistolMesh->GetSocketRotation("Muzzle"));
		PlayFireFX();
		PlaySlideBack();
		bSlideBack = true;
		ChamberedRound = nullptr;

		AttemptCharge();
	}
	else
	{
		// Play SFX of hammer snapping, ie. failure to fire
	}
}
void ASigPistol::AttemptCharge()
{
	if (bSlideBack && LoadedMagazine && LoadedMagazine->GetCurrentCapacity() > 0)
	{
		LoadedMagazine->ExpendCartridge();
		ChamberedRound = LoadedMagazine->GetLoadedAmmunition();
		PlaySlideForward();
		bSlideBack = false;

	}

}
void ASigPistol::AttachMag()
{
	LoadedMagazine->GetPickupMesh()->SetSimulatePhysics(false);
	LoadedMagazine->GetPickupMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LoadedMagazine->SetPickupEnabled(false);
	LoadedMagazine->AttachToComponent(PistolMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "MagazineWell");

}
void ASigPistol::DropMag()
{
	LoadedMagazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	LoadedMagazine->GetPickupMesh()->SetSimulatePhysics(true);
	LoadedMagazine->GetPickupMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LoadedMagazine->SetPickupEnabled(true);
	LoadedMagazine = nullptr;

}
