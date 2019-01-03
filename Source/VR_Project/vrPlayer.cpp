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
	LeftController->SetupAttachment(vrRoot); 
	LeftController->MotionSource = "Left";

	RightController = CreateDefaultSubobject<UMotionControllerComponent>("Right Controller");
	RightController->SetupAttachment(vrRoot);
	RightController->MotionSource = "Right";

	LeftVolume = CreateDefaultSubobject<USphereComponent>("Left Pickup Scan Volume");
	LeftVolume->SetupAttachment(LeftController);

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
	PlayerInputComponent->BindAction("LGrip", IE_Released, this, &AvrPlayer::LeftGripPull);

}
void AvrPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}
void AvrPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		auto HeadForwardRot = HeadsetCamera->GetComponentRotation();
		HeadForwardRot.Pitch = 0.f;
		HeadForwardRot.Roll = 0.f;
		auto HeadForward = HeadForwardRot.Vector();

		AddMovementInput(HeadForward, Value);
	}
}
void AvrPlayer::MoveRight(float Value)
{
	if (Value != 0)
	{
		auto HeadForwardRot = HeadsetCamera->GetComponentRotation();
		HeadForwardRot.Pitch = 0.f;
		HeadForwardRot.Roll = 0.f;
		HeadForwardRot.Yaw += 90.f;
		auto HeadForward = HeadForwardRot.Vector();

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
	ExecuteGrip(LeftScanTarget, LeftHeldObject, LeftController);
}
void AvrPlayer::LeftGripRelease()
{
	ExecuteDrop(LeftHeldObject);
}

// Interaction Execution
void AvrPlayer::ExecuteGrip(AvrPickup* &ScanObject, AvrPickup* &GrippedObjectPointer, UMotionControllerComponent* GrabbingMC)
{
	if (ScanObject)
	{
		GrippedObjectPointer = ScanObject;
		//GrippedObjectPointer->SnapTo(GrabbingMC);
		ScanObject = nullptr;
	}
}
void AvrPlayer::ExecuteDrop(AvrPickup *& ObjectToDrop)
{
	if (ObjectToDrop)
	{
		// ObjectToDrop->DropFrom(this);
		// ObjectToDrop = nullptr;
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
	// TODO: De-activate highlight animation or effect.
}
