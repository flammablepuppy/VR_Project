// Copyright Aviator Games 2019

#include "vrPlayer.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "vrPickup.h"
#include "Components/CapsuleComponent.h"
#include "HealthStats.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineGlobals.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "vrBelt.h"

AvrPlayer::AvrPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;

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

	HealthStatsComp = CreateDefaultSubobject<UHealthStats>("Health and Stats Component");

	UtilityBelt = CreateDefaultSubobject<UvrBelt>("Utility Belt");
	UtilityBelt->SetupAttachment(RootComponent);
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

	PlayerInputComponent->BindAction("ResetLevel", IE_Pressed, this, &AvrPlayer::ResetTestingMap);

}
void AvrPlayer::BeginPlay()
{
	Super::BeginPlay();

	/** Initialize for use with MotionInput */
	HeadLastRelPos = HeadsetCamera->GetComponentLocation();
	LeftLastRelPos = LeftController->GetComponentLocation();
	RightLastRelPos = RightController->GetComponentLocation();

	/** Prevent MotionImput from firing anything for a moment after starting */
	GetWorldTimerManager().SetTimer(SprintLeft_Timer, SprintCooldownDuration, false);
	GetWorldTimerManager().SetTimer(SprintRight_Timer, SprintCooldownDuration, false);

}
void AvrPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OffsetRoot();
	if (!HealthStatsComp->bOwnerIsDead) { MotionInputScan(); }

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
		if (Value > 0.3f)
		{
			bHasForwardMovementInput = true;
		}
	}
	else
	{
		bHasForwardMovementInput = false;
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
void AvrPlayer::SnapTurn(float Value)
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
void AvrPlayer::MotionInputScan()
{
	//	Using the point on the ground in the center of the pawn collision, get the relative position of the head and hands
	FVector MeasureingPoint = GetCapsuleComponent()->GetComponentLocation(); MeasureingPoint.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	HeadRelative = HeadsetCamera->GetComponentTransform().InverseTransformPosition(MeasureingPoint);
	LeftRelative = LeftController->GetComponentTransform().InverseTransformPosition(MeasureingPoint);
	RightRelative = RightController->GetComponentTransform().InverseTransformPosition(MeasureingPoint);

	//	With those relative positions, measure each components relative velocity based off of their positions last tick (set at the end of this function)
	HeadRelVel = HeadLastRelPos - HeadRelative;
	LeftRelVel = LeftLastRelPos - LeftRelative;
	RightRelVel = RightLastRelPos - RightRelative;

		/** IMPACT DAMAGE
		*	Damage character for abruput velocity changes
		*/
		if ((VelocityLastTick - GetVelocity()).Size() > VelocityChangeDamageSpeed)
		{
			ApplyImpactDamage();
		}
		
		/** MOTION JUMP
		*	Trigger jump when conditions are met
		*	High jump with forward impulse when arms swing and head pops
		*	Small jump when arms are whipped up quickly while moving forward
		*/
		if (HeadRelVel.Z > JumpHeadReqZ && LeftRelVel.Z > JumpHandReqZ && RightRelVel.Z > JumpHandReqZ)
		{
			FTimerHandle FiringJump_Timer;
			GetWorldTimerManager().SetTimer(FiringJump_Timer, this, &AvrPlayer::MotionJump, JumpDurationReq, false);

			// Prevent triggering sprint while jumping is being checked
			GetWorldTimerManager().SetTimer(SprintLeft_Timer, JumpDurationReq, false);
			GetWorldTimerManager().SetTimer(SprintRight_Timer, JumpDurationReq, false);

		}
		else if (LeftRelVel.Z > JumpSmallReq && RightRelVel.Z > JumpSmallReq && bHasForwardMovementInput)
		{
			GetCharacterMovement()->JumpZVelocity = 550.f;
			Jump();
		}

		/** MOTION SPRINT
		*	Add an impulse in the direction the forward arm swings
		*	Triggered when the forward swinging arm x and y relative velocities combined exceed a threshold while the other hand is going in reverse by half that speed
		*/
		if (LeftRelVel.X + LeftRelVel.Z > SprintMinImpulseSpeed && 
			!GetWorldTimerManager().IsTimerActive(SprintLeft_Timer) && 
			!GetCharacterMovement()->IsFalling() &&
			RightRelVel.Z + RightRelVel.X < -SprintMinImpulseSpeed * 0.5f)
		{
			GetWorldTimerManager().SetTimer(SprintLeft_Timer, SprintCooldownDuration, false);
			GetWorldTimerManager().SetTimer(SprintDecelReset_Timer, SprintDecelResetDuration, false);

			FVector Direction = LeftController->GetComponentLocation() - SprintLeftLastPos;
			Direction.Z = 0.f;
			FRotator DirRot = Direction.ToOrientationRotator();
			DirRot -= FRotator(0.f, 10.f, 0.f); // To help offset the way the arm naturally swings
			Direction = DirRot.Vector();
			Direction.GetSafeNormal();

			float Percent = LeftRelVel.X + LeftRelVel.Z - SprintMinImpulseSpeed / SprintMaxImpulseSpeed;
			if (Percent > 1.f) { Percent = 1.f; }

			MotionSprint(Direction, Percent);
		}
		if (RightRelVel.X + RightRelVel.Z > SprintMinImpulseSpeed && 
			!GetWorldTimerManager().IsTimerActive(SprintRight_Timer) && 
			!GetCharacterMovement()->IsFalling() &&
			LeftRelVel.Z + LeftRelVel.X < -SprintMinImpulseSpeed * 0.5f)
		{
			GetWorldTimerManager().SetTimer(SprintRight_Timer, SprintCooldownDuration, false);
			GetWorldTimerManager().SetTimer(SprintDecelReset_Timer, SprintDecelResetDuration, false);

			FVector Direction = RightController->GetComponentLocation() - SprintRightLastPos;
			Direction.Z = 0.f;
			FRotator DirRot = Direction.ToOrientationRotator();
			DirRot += FRotator(0.f, 10.f, 0.f); // To help offset the way the arm naturally swings
			Direction = DirRot.Vector();
			Direction.GetSafeNormal();

			float Percent = RightRelVel.X + RightRelVel.Z - SprintMinImpulseSpeed / SprintMaxImpulseSpeed;
			if (Percent > 1.f) { Percent = 1.f; }

			MotionSprint(Direction, Percent);
		}
		if (!GetWorldTimerManager().IsTimerActive(SprintDecelReset_Timer))
		{
			GetCharacterMovement()->GroundFriction = 2.f;
			GetCharacterMovement()->MaxWalkSpeed = 300.f;
		}

	//	Remember positions from this tick for use in the next tick
	HeadLastRelPos = HeadRelative;
	LeftLastRelPos = LeftRelative;
	RightLastRelPos = RightRelative;
	SprintLeftLastPos = LeftController->GetComponentLocation();
	SprintRightLastPos = RightController->GetComponentLocation();

	//	Remember pawn velocity to check for impact damage next tick
	VelocityLastTick = GetMovementComponent()->Velocity;

}
void AvrPlayer::ApplyImpactDamage()
{
	float AppliedDamage = (VelocityLastTick - GetVelocity()).Size() - VelocityChangeDamageSpeed;
	AppliedDamage *= ExponentialImpactDamage;
	AppliedDamage *= AppliedDamage; // Having the damage be exponential makes it feel much more fair. Big falls hurt a lot, little ones not so much

	// Damage is always at least this amount
	if (AppliedDamage < MinimumImpactDamage)
	{
		AppliedDamage = MinimumImpactDamage;
	}

	// Hard impacts cause player to drop anything they're holding
	if (AppliedDamage > 65.f) 
	{
		if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
		{
			ExecuteDrop(LeftHeldObject);
		}
		if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
		{
			ExecuteDrop(RightHeldObject);
		}

		UGameplayStatics::ApplyDamage(this, AppliedDamage, this->GetController(), this, MotionDamage);
	}
}
void AvrPlayer::MotionJump()
{
	if (HeadRelVel.Z > JumpHeadReqZ/2)
	{
		GetCharacterMovement()->JumpZVelocity = 840.f;
		Jump();

		if (bHasForwardMovementInput)
		{
			FVector ForwardImpulse = GetHeadsetCam()->GetForwardVector();
			ForwardImpulse.Z = 0.f;
			ForwardImpulse.GetSafeNormal();
			ForwardImpulse *= JumpBigForwardImpulse;

			GetCharacterMovement()->Velocity += ForwardImpulse;
			GetCharacterMovement()->UpdateComponentVelocity();
		}
	}
}
void AvrPlayer::MotionSprint(FVector ImpulseDirection, float ImpulsePercent)
{
	// Lower ground friction to simulate higher momentum
	GetCharacterMovement()->GroundFriction = 0.18f;

	// Increase walk speed 
	float SpeedIncrement = (SprintMaxSpeed - SprintMinSpeed) / 3.f;
	float SpeedAddition = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed + SpeedIncrement, SprintMinSpeed, SprintMaxSpeed);
	GetCharacterMovement()->MaxWalkSpeed = SpeedAddition;	

	// Apply an impulse
	GetCharacterMovement()->Velocity += (ImpulseDirection * ImpulsePercent * SpeedIncrement);
	if (GetCharacterMovement()->Velocity.Size() > SprintMaxSpeed)
	{
		GetCharacterMovement()->Velocity *= (SprintMaxSpeed + (SprintMinSpeed * 0.25)) / GetCharacterMovement()->Velocity.Size();
	}
	GetCharacterMovement()->UpdateComponentVelocity();
}
UMotionControllerComponent * AvrPlayer::GetForwardController()
{
	// Works on distance to capsule component rather than X position, this takes the 
	if ((LeftController->GetComponentLocation() - GetActorLocation()).Size() > (RightController->GetComponentLocation() - GetActorLocation()).Size())
	{
		return LeftController;
	}
	else
	{
		return RightController;
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

void AvrPlayer::ResetTestingMap()
{
	UGameplayStatics::OpenLevel(GetWorld(), LevelToLoad);

}

// Interaction Execution
void AvrPlayer::ExecuteGrip(AvrPickup* &ScanObject, AvrPickup* &GrippedObjectPointer, UMotionControllerComponent* GrabbingMC)
{
	if (ScanObject)
	{
		GrippedObjectPointer = ScanObject;
		GrippedObjectPointer->SnapInitiate(GrabbingMC);
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
