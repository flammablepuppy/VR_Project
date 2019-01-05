// Fill out your copyright notice in the Description page of Project Settings.

#include "vrPlayer.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "vrPickup.h"

AvrPlayer::AvrPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = true;

	vrRoot = CreateDefaultSubobject<USceneComponent>("vrRoot");
	vrRoot->SetupAttachment(RootComponent);

	HeadsetCamera = CreateDefaultSubobject<UCameraComponent>("Headset Camera");
	HeadsetCamera->SetupAttachment(vrRoot);

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>("Left Controller");
	LeftController->SetupAttachment(HeadsetCamera); 
	LeftController->MotionSource = "Left";

	LeftVolume = CreateDefaultSubobject<USphereComponent>("Left Pickup Scan Volume");
	LeftVolume->SetupAttachment(LeftController);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>("Right Controller");
	RightController->SetupAttachment(HeadsetCamera);
	RightController->MotionSource = "Right";

	RightVolume = CreateDefaultSubobject<USphereComponent>("Right Pickup Scan Volume");
	RightVolume->SetupAttachment(RightController);

}
void AvrPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// LeftVolume->OnComponentBeginOverlap.BindDynamic( TODO: Finsh binding overlap and endoverlap events for both volumes to show when a pickup is in range
	// RightVolume->OnComponentBeginOverlap.BindDynamic( 

	PlayerInputComponent->BindAxis("MoveForward", this, &AvrPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AvrPlayer::MoveRight);
	PlayerInputComponent->BindAxis("MouseLookPitch", this, &AvrPlayer::MouseLookPitch);
	PlayerInputComponent->BindAxis("MouseLookYaw", this, &AvrPlayer::MouseLookYaw);

	PlayerInputComponent->BindAction("LGrip", IE_Pressed, this, &AvrPlayer::LeftGripPull);
	PlayerInputComponent->BindAction("LGrip", IE_Released, this, &AvrPlayer::LeftGripRelease);
	PlayerInputComponent->BindAction("RGrip", IE_Pressed, this, &AvrPlayer::RightGripPull);
	PlayerInputComponent->BindAction("RGrip", IE_Released, this, &AvrPlayer::RightGripRelease);

	//PlayerInputComponent->BindAction("LTrigger", IE_Pressed, this, &AvrPlayer::LeftTriggerPull);
	//PlayerInputComponent->BindAction("LTrigger", IE_Released, this, &AvrPlayer::LeftTriggerRelease);
	//PlayerInputComponent->BindAction("RTrigger", IE_Pressed, this, &AvrPlayer::RightTriggerPull);
	//PlayerInputComponent->BindAction("RTrigger", IE_Released, this, &AvrPlayer::RightTriggerRelease);

	PlayerInputComponent->BindAxis("LTrig", this, &AvrPlayer::LeftTriggerHandle);
	PlayerInputComponent->BindAxis("RTrig", this, &AvrPlayer::RightTriggerHandle);

	PlayerInputComponent->BindAction("LTop", IE_Pressed, this, &AvrPlayer::LeftTopPush);
	PlayerInputComponent->BindAction("LTop", IE_Released, this, &AvrPlayer::LeftTopRelease);
	PlayerInputComponent->BindAction("RTop", IE_Pressed, this, &AvrPlayer::RightTopPush);
	PlayerInputComponent->BindAction("RTop", IE_Released, this, &AvrPlayer::RightTopRelease);

	PlayerInputComponent->BindAction("LBottom", IE_Pressed, this, &AvrPlayer::LeftBottomPush);
	PlayerInputComponent->BindAction("LBottom", IE_Released, this, &AvrPlayer::LeftBottomRelease);
	PlayerInputComponent->BindAction("RBottom", IE_Pressed, this, &AvrPlayer::RightBottomPush);
	PlayerInputComponent->BindAction("RBottom", IE_Released, this, &AvrPlayer::RightBottomRelease);

}
void AvrPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}
void AvrPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//OffsetRoot();
}
 
// VR Functions
void AvrPlayer::OffsetRoot()
{
	FVector HeadDelta = GetActorLocation() - HeadsetCamera->GetComponentLocation();
	AddActorWorldOffset(HeadDelta);
	vrRoot->AddWorldOffset(-HeadDelta);
}

// Locomotion Functions
void AvrPlayer::MoveForward(float Value)
{
	if (Value != 0)
	{
		FRotator HeadForwardRot = HeadsetCamera->GetComponentRotation();
		HeadForwardRot.Pitch = 0.f;
		HeadForwardRot.Roll = 0.f;
		FVector HeadForward = HeadForwardRot.Vector();

		AddMovementInput(HeadForward, Value);
	}
}
void AvrPlayer::MoveRight(float Value)
{
	if (Value != 0)
	{
		FRotator HeadForwardRot = HeadsetCamera->GetComponentRotation();
		HeadForwardRot.Pitch = 0.f;
		HeadForwardRot.Roll = 0.f;
		HeadForwardRot.Yaw += 90.f;
		FVector HeadForward = HeadForwardRot.Vector();

		AddMovementInput(HeadForward, Value);
	}
}
void AvrPlayer::MouseLookPitch(float Value)
{
	if (Value != 0 && bMouseEnabled)
	{
		AddControllerPitchInput(Value);
	}
}
void AvrPlayer::MouseLookYaw(float Value)
{
	if (Value != 0 && bMouseEnabled)
	{
		AddControllerYawInput(Value);
	}
}

// Interaction Calls
void AvrPlayer::LeftGripPull()
{
	ScanForClosestObject(LeftVolume, LeftScanTarget, LeftController);
	ExecuteGrip(LeftScanTarget, LeftHeldObject, LeftController);
}
void AvrPlayer::LeftGripRelease()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		ExecuteDrop(LeftHeldObject);
	}
}
void AvrPlayer::LeftTriggerHandle(float Value)
{
	if (Value > 0)
	{
		LeftTriggerPull(Value);
	}
}
void AvrPlayer::LeftTriggerPull(float Value)
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->TriggerPulled(Value);
}
void AvrPlayer::LeftTriggerRelease()
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->TriggerReleased();
}
void AvrPlayer::LeftTopPush()
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->TopPushed();
}
void AvrPlayer::LeftTopRelease()
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->TopReleased();
}
void AvrPlayer::LeftBottomPush()
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->BottomPushed();
}
void AvrPlayer::LeftBottomRelease()
{
	if (!LeftHeldObject) { return; }
	LeftHeldObject->BottomReleased();
}

void AvrPlayer::RightGripPull()
{
	ScanForClosestObject(RightVolume, RightScanTarget, RightController);
	ExecuteGrip(RightScanTarget, RightHeldObject, RightController);
}
void AvrPlayer::RightGripRelease()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		ExecuteDrop(RightHeldObject);
	}
}
void AvrPlayer::RightTriggerHandle(float Value)
{
	if (Value > 0)
	{
		RightTriggerPull(Value);
	}
}
void AvrPlayer::RightTriggerPull(float Value)
{
	if (!RightHeldObject) { return; }
	RightHeldObject->TriggerPulled(Value);
}
void AvrPlayer::RightTriggerRelease()
{
	if (!RightHeldObject) { return; }
	RightHeldObject->TriggerReleased();
}
void AvrPlayer::RightTopPush()
{
	if (!RightHeldObject) { return; }
	RightHeldObject->TopPushed();
}
void AvrPlayer::RightTopRelease()
{
	if (!RightHeldObject) { return; }
	RightHeldObject->TopReleased();
}
void AvrPlayer::RightBottomPush()
{
	if (!RightHeldObject) { return; }
	RightHeldObject->BottomPushed();
}
void AvrPlayer::RightBottomRelease()
{
	if (!RightHeldObject) { return; }
	RightHeldObject->BottomReleased();
}

// Interaction Execution
void AvrPlayer::ExecuteGrip(AvrPickup* &ScanObject, AvrPickup* &GrippedObjectPointer, UMotionControllerComponent* GrabbingMC)
{
	if (ScanObject)
	{
		GrippedObjectPointer = ScanObject;
		GrippedObjectPointer->SnapTo(GrabbingMC);
		ScanObject = nullptr;
	}
}
void AvrPlayer::ExecuteDrop(AvrPickup *& ObjectToDrop)
{
	if (ObjectToDrop)
	{
		ObjectToDrop->Drop();
		ObjectToDrop = nullptr;
	}
}
void AvrPlayer::ScanForClosestObject(USphereComponent * VolumeToScan, AvrPickup* &ScanRef, UMotionControllerComponent* MotionController)
{
	ScanRef = nullptr;

	TSet<AActor*> ScannedActors;
	VolumeToScan->GetOverlappingActors(ScannedActors);

	for (AActor* Pickup : ScannedActors)
	{
		AvrPickup* ValidPickup = Cast<AvrPickup>(Pickup);
		if (ValidPickup && !ScanRef)
		{
			ScanRef = ValidPickup;
		}
		else if (ValidPickup)
		{
			float PickupDistance = (ValidPickup->GetActorLocation() - MotionController->GetComponentLocation()).Size();
			float CurrentScanDistance = (ScanRef->GetActorLocation() - MotionController->GetComponentLocation()).Size();
			if (PickupDistance < CurrentScanDistance)
			{
				ScanRef = ValidPickup;
			}
		}
	}
}

// Interaction FX Functions
void AvrPlayer::BeginGrabHighlight(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	auto ValidPickup = Cast<AvrPickup>(OtherActor);
	if (ValidPickup)
	{
		// TODO: Activate an animation or highlight effect.
	}
}
void AvrPlayer::EndGrabHighlight(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	TSet<AActor*> ScannedActors;
	OverlappedComponent->GetOverlappingActors(ScannedActors);

	if (ScannedActors.Num() < 1)
	{
		// TODO: De-activate highlight animation or effect.
	}
}
