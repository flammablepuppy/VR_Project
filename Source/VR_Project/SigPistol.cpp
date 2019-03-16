// Copyright Aviator Games 2019

#include "SigPistol.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "vrProjectile.h"
#include "Components/SphereComponent.h"
#include "WeaponMag.h"

ASigPistol::ASigPistol()
{
	PickupMesh->SetVisibility(false);

	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Pistol Skeletal Mesh");
	PistolMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PistolMesh->SetupAttachment(RootComponent);

	MagazineLoadSphere = CreateDefaultSubobject<USphereComponent>("Magazine Load Sphere");
	MagazineLoadSphere->SetupAttachment(PistolMesh);
	
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

	if (bMagInTransit)
	{
		MoveMagToWell();
	}

}
void ASigPistol::MagOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (LoadedMagazine) { return; }

	// Check for a valid magazine, and that the magazine is currently being held
	AWeaponMag* Magazine = Cast<AWeaponMag>(OtherActor);
	if (Magazine && Magazine->GetOwningMC() && OwningMC)
	{
		Magazine->Drop();
		LoadedMagazine = Magazine;

		LoadedMagazine->GetPickupMesh()->SetSimulatePhysics(false);
		LoadedMagazine->GetPickupMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LoadedMagazine->SetPickupEnabled(false);
		LoadedMagazine->AttachToComponent(PistolMesh, FAttachmentTransformRules::KeepWorldTransform);

		bMagInTransit = true;

	}

}

// Input Functions
void ASigPistol::Drop()
{
	Super::Drop();

}
void ASigPistol::TriggerPulled(float Value)
{
	if (Value > 0.3f && !bTriggerPulled && !bMagInTransit)
	{
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
void ASigPistol::MoveMagToWell()
{
	if (!LoadedMagazine) { return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FTransform MagTransform = LoadedMagazine->GetTransform();
	FTransform WellTransform = PistolMesh->GetSocketTransform("MagazineWell");
	
	FVector DeltaLocation = (WellTransform.GetLocation() - MagTransform.GetLocation()) * DeltaTime / MagazineLoadTime;
	FQuat DeltaRotation = (WellTransform.GetRotation() - MagTransform.GetRotation()) * DeltaTime / MagazineLoadTime;

	LoadedMagazine->SetActorLocation(MagTransform.GetLocation() + DeltaLocation);
	LoadedMagazine->SetActorRotation(MagTransform.GetRotation() + DeltaRotation);

	if (DeltaLocation.IsNearlyZero(0.1f))
	{
		AttachMag();
		bMagInTransit = false;
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
