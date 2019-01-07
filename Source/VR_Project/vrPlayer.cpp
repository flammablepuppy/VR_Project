// Fill out your copyright notice in the Description page of Project Settings.

#include "vrPlayer.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "vrPickup.h"
#include "Components/CapsuleComponent.h"

AvrPlayer::AvrPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	vrRoot = CreateDefaultSubobject<USceneComponent>("vrRoot");
	vrRoot->SetupAttachment(RootComponent);

	HeadsetCamera = CreateDefaultSubobject<UCameraComponent>("Headset Camera");
	HeadsetCamera->SetupAttachment(vrRoot);

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>("Left Controller");
	LeftController->SetupAttachment(vrRoot); 
	LeftController->MotionSource = "Left";

	LeftVolume = CreateDefaultSubobject<USphereComponent>("Left Pickup Scan Volume");
	LeftVolume->SetupAttachment(LeftController);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>("Right Controller");
	RightController->SetupAttachment(vrRoot);
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
	PlayerInputComponent->BindAxis("SnapTurn", this, &AvrPlayer::SnapTurn);

	PlayerInputComponent->BindAction("LGrip", IE_Pressed, this, &AvrPlayer::LeftGripPull);
	PlayerInputComponent->BindAction("LGrip", IE_Released, this, &AvrPlayer::LeftGripRelease);
	PlayerInputComponent->BindAction("RGrip", IE_Pressed, this, &AvrPlayer::RightGripPull);
	PlayerInputComponent->BindAction("RGrip", IE_Released, this, &AvrPlayer::RightGripRelease);

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

	OffsetRoot();
}
 
// VR Functions
void AvrPlayer::OffsetRoot()
{
	FVector HeadDelta = HeadsetCamera->GetComponentLocation() - GetActorLocation();
	HeadDelta.Z = 0.f;
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
		FRotator NewRot = vrRoot->GetComponentRotation();
		NewRot -= FRotator(Value, 0.f, 0.f);
		vrRoot->SetWorldRotation(NewRot);
	}
}
void AvrPlayer::MouseLookYaw(float Value)
{
	if (Value != 0 && bMouseEnabled)
	{
		AddControllerYawInput(Value);
	}
}
void AvrPlayer::SnapTurn(float Value) // TODO: Make a tick function that rapidly rotates the veiw to give better indication of where you turned to
{
	if (Value > 0.1f && bSnapTurnReady)
	{
		FRotator LookDirection = GetCapsuleComponent()->GetComponentRotation();
		LookDirection += FRotator(0.f, SnapTurnIncrement, 0.f);
		GetCapsuleComponent()->SetWorldRotation(LookDirection);
		bSnapTurnReady = false;
	}
	if (Value < -0.1f && bSnapTurnReady)
	{
		FRotator LookDirection = GetCapsuleComponent()->GetComponentRotation();
		LookDirection -= FRotator(0.f, SnapTurnIncrement, 0.f);
		GetCapsuleComponent()->SetWorldRotation(LookDirection);
		bSnapTurnReady = false;
	}
	if (Value < 0.1 && Value > -0.1 && !bSnapTurnReady)
	{
		bSnapTurnReady = true;
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
	if (!LeftHeldObject) { return; }
	if (Value > 0 && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TriggerPulled(Value);
	}
}
void AvrPlayer::LeftTopPush()
{
	if (!LeftHeldObject) { return; }
	if (LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TopPushed();
	}
}
void AvrPlayer::LeftTopRelease()
{
	if (!LeftHeldObject) { return; }
	if (LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TopReleased();
	}
}
void AvrPlayer::LeftBottomPush()
{
	if (!LeftHeldObject) { return; }
	if (LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->BottomPushed();
	}
}
void AvrPlayer::LeftBottomRelease()
{
	if (!LeftHeldObject) { return; }
	if (LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->BottomReleased();
	}
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
	if (!RightHeldObject) { return; }
	if (Value > 0 && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TriggerPulled(Value);
	}
}
void AvrPlayer::RightTopPush()
{
	if (!RightHeldObject) { return; }
	if (RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TopPushed();
	}
}
void AvrPlayer::RightTopRelease()
{
	if (!RightHeldObject) { return; }
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TopReleased();
	}
}
void AvrPlayer::RightBottomPush()
{
	if (!RightHeldObject) { return; }
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->BottomPushed();
	}
}
void AvrPlayer::RightBottomRelease()
{
	if (!RightHeldObject) { return; }
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->BottomReleased();
	}
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

// Setters
void AvrPlayer::SetMouseEnabled(bool NewState)
{
	bMouseEnabled = NewState;
}
