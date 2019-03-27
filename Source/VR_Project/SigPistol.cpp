// Copyright Aviator Games 2019

#include "SigPistol.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "vrProjectile.h"
#include "Components/SphereComponent.h"
#include "WeaponMag.h"
#include "TimerManager.h"
#include "MagCartridge.h"
#include "vrPlayer.h"
#include "vrHolster.h"

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
		LoadedMagazine->SnapInitiate(PistolMesh, "MagazineWell");
		LoadedMagazine->GetPickupMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Normally taken care of by the magazine in SnapOn
		LoadedMagazine->SetActorLocation(PistolMesh->GetSocketLocation("MagazineWell"));
		LoadedMagazine->SetActorRotation(PistolMesh->GetSocketRotation("MagazineWell"));

		ChamberedRound = LoadedMagazine->GetCompatibleCartridge();

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
	//UE_LOG(LogTemp, Warning, TEXT("Component detected: %s"), *OtherComp->GetClass()->GetFName().ToString())

	UStaticMeshComponent* OverlappedMesh = Cast<UStaticMeshComponent>(OtherComp);
	if (OverlappedMesh)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Mesh detected"))

		AWeaponMag* OverlappedMag = Cast<AWeaponMag>(OverlappedMesh->GetOwner());
		if (OverlappedMag && OverlappedMag->GetOwningMC() && OwningMC)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Magazine detected"))

			OverlappedMag->OnDrop.Clear();
			OverlappedMag->Drop();
			OverlappedMag->SnapInitiate(PistolMesh, "MagazineWell");
			LoadedMagazine = OverlappedMag;
			BP_PlayMagLoad();
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

		if (ChamberedRound)
		{
			AMagCartridge* DroppedCartridge =
			GetWorld()->SpawnActor<AMagCartridge>(ChamberedRound, PistolMesh->GetSocketLocation("EjectionPort"), PistolMesh->GetSocketRotation("EjectionPort"));

			FVector EjectDirection = PickupMesh->GetRightVector();
			EjectDirection.Z += 0.5f;
			DroppedCartridge->GetPickupMesh()->AddImpulse(EjectDirection * 2.f);

			ChamberedRound = nullptr;
		}
	}
}
void ASigPistol::BottomPushed()
{
	if (LoadedMagazine)
	{
		LoadedMagazine->Drop();
		LoadedMagazine->SetActorRelativeLocation(PickupMesh->GetSocketLocation("Muzzle") + (PickupMesh->GetUpVector() * -10.f));
		BPBottomPush();
		LoadedMagazine = nullptr;
	}

}

// Functionality
void ASigPistol::DischargeRound()
{
	if (!bSlideBack && ChamberedRound)
	{
		AMagCartridge* Cart = ChamberedRound->GetDefaultObject<AMagCartridge>();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = this;

		GetWorld()->SpawnActor<AvrProjectile>(Cart->GetProjectile(), 
			PistolMesh->GetSocketLocation("Muzzle") + (PickupMesh->GetForwardVector() * (GetOwningPlayer()->GetVelocity().Size() * 0.05f)), 
			PistolMesh->GetSocketRotation("Muzzle"), 
			SpawnParams);

		PlayFireFX();
		PlaySlideBack();
		bSlideBack = true;

		AvrPickup* DroppedCasing =
		GetWorld()->SpawnActor<AvrPickup>(Cart->GetCasing(), PistolMesh->GetSocketLocation("EjectionPort"), PistolMesh->GetSocketRotation("EjectionPort"));

		ChamberedRound = nullptr;

		FVector EjectDirection = PickupMesh->GetRightVector();
		EjectDirection.Z += 0.5f;
		DroppedCasing->GetPickupMesh()->AddImpulse(EjectDirection * 5.f);

		FTimerHandle Charge;
		float DetlaSecond = GetWorld()->GetDeltaSeconds();
		GetWorldTimerManager().SetTimer(Charge, this, &ASigPistol::AttemptCharge, DetlaSecond * 2.f);
	}
	else
	{
		BP_PlayHammer();
	}
}
void ASigPistol::AttemptCharge()
{
	if (bSlideBack && LoadedMagazine && LoadedMagazine->GetCurrentCapacity() > 0)
	{
		LoadedMagazine->ExpendCartridge();
		ChamberedRound = LoadedMagazine->GetCompatibleCartridge();
	}
	if (ChamberedRound)
	{
		PlaySlideForward();
		bSlideBack = false;
	}
}
