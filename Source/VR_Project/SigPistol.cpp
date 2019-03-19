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

	if (bSpawnsLoaded && CompatibleMagazine)
	{
		LoadedMagazine = GetWorld()->SpawnActor<AWeaponMag>(CompatibleMagazine);
		LoadedMagazine->GetPickupMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LoadedMagazine->SetActorLocation(PistolMesh->GetSocketLocation("MagazineWell"));
		LoadedMagazine->SetActorRotation(PistolMesh->GetSocketRotation("MagazineWell"));
		LoadedMagazine->SnapInitiate(PistolMesh, "MagazineWell");

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

	UE_LOG(LogTemp, Warning, TEXT("Pistol calls MagOverlap"))

	if (OtherActor->IsA(CompatibleMagazine))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pistol detects CompatibleMagzine"))

		AWeaponMag* Magazine = Cast<AWeaponMag>(OtherActor);
		if (Magazine && Magazine->GetOwningMC() && OwningMC) // Cast to access class functions, check if the mag is being held and this pistol is being held
		{
			UE_LOG(LogTemp, Warning, TEXT("Pistol casts Magzine successfully"))

			Magazine->Drop();
			Magazine->SnapInitiate(PistolMesh, "MagazineWell");
			LoadedMagazine = Magazine;
		}
	}
}

// Input Functions
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
void ASigPistol::BottomPushed()
{
	if (LoadedMagazine)
	{
		LoadedMagazine->Drop();
		LoadedMagazine = nullptr;
	}

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
