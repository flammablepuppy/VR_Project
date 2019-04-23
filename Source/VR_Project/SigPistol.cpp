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
#include "vrBelt.h"
#include "TimerManager.h"
#include "MotionControllerComponent.h"

ASigPistol::ASigPistol()
{
	PickupMesh->SetVisibility(false);

	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Pistol Skeletal Mesh");
	PistolMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PistolMesh->SetupAttachment(RootComponent);

	MagazineLoadSphere = CreateDefaultSubobject<USphereComponent>("Magazine Load Sphere");
	MagazineLoadSphere->SetupAttachment(PistolMesh);

	if (bTwoHandEnabled)
	{
		WeaponForegrip = CreateDefaultSubobject<USphereComponent>("Weapon Foregrip");
		WeaponForegrip->SetupAttachment(RootComponent);

	}
	
}
void ASigPistol::BeginPlay()
{
	Super::BeginPlay();

	MagazineLoadSphere->OnComponentBeginOverlap.AddDynamic(this, &ASigPistol::MagOverlap);
	//if (bTwoHandEnabled)
	//{
	//	WeaponForegrip->OnComponentBeginOverlap.AddDynamic(this, &ASigPistol::ForegripSub);
	//}

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

/**
*
*/

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

			OverlappedMag->SetLoading(true);
			OverlappedMag->OnDrop.Clear();
			OverlappedMag->Drop();
			OverlappedMag->SnapInitiate(PistolMesh, "MagazineWell");

			LoadedMagazine = OverlappedMag;
			BP_PlayMagLoad();
		}
	}
}
void ASigPistol::ForegripSub(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UMotionControllerComponent* MCOverlap = Cast<UMotionControllerComponent>(OtherComp);
	if (MCOverlap)
	{
		AvrPlayer* vrPlayer = Cast<AvrPlayer>(MCOverlap->GetOwner());
		if (vrPlayer)
		{
			vrPlayer->OnGrip.AddUniqueDynamic(this, &ASigPistol::GrabForegrip);
		}
	}
}
void ASigPistol::GrabForegrip(UMotionControllerComponent* RequestingController)
{
	FVector NewDirection = RequestingController->GetComponentLocation() - GetActorLocation();
	FRotator NewRotation = NewDirection.Rotation();
	NewRotation.Roll = GetOwningMC()->GetComponentRotation().Roll;

	SetActorRotation(NewRotation);
}

// Input Functions
void ASigPistol::TriggerPulled(float Value)
{
	if (bAutomaticFire)
	{
		if (Value > 0.3f)
		{
			if (!GetWorldTimerManager().IsTimerActive(AutoCooldown_Timer))
			{
				GetWorldTimerManager().SetTimer(AutoCooldown_Timer, this, &ASigPistol::DischargeRound, AutoCooldown, true, 0.f);
			}
		}
		if (Value < 0.3f)
		{
			GetWorldTimerManager().ClearTimer(AutoCooldown_Timer);
		}
	}
	else
	{
		if (Value > 0.3f && !bTriggerPulled)
		{
			bTriggerPulled = true;
			DischargeRound();
		}
		else if (Value < 0.3f)
		{
			bTriggerPulled = false;
		}
	}
}
/**
*	Used to manipulate the slide forward and back
*	With a magazine loaded it will chamber a round from the magazine
*	When the slide is forward and brought back it will eject a cartridge if it's chambered
*/
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
		LoadedMagazine->OnDrop.Clear();

		LoadedMagazine->SetMagSearchForHolster(OwningPlayer);

		LoadedMagazine->Drop();
		LoadedMagazine->SetActorRelativeLocation(PickupMesh->GetSocketLocation("Muzzle") + (PickupMesh->GetUpVector() * -10.f));

		BPBottomPush();
		LoadedMagazine = nullptr;
	}
}

void ASigPistol::Drop()
{
	Super::Drop();

	if (GetWorldTimerManager().IsTimerActive(AutoCooldown_Timer))
	{
		GetWorldTimerManager().ClearTimer(AutoCooldown_Timer);
	}
}

// Functionality
void ASigPistol::DischargeRound()
{
	if (!OwningPlayer || !OwningMC) { return; }

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
		DroppedCasing->SetLifeSpan(10.f);

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
