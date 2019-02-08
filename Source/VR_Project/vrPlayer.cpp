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

	BaseCharacterSpeed = GetCharacterMovement()->MaxWalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = BaseCharacterSpeed * 1.5f;

}
void AvrPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OffsetRoot();
	MotionInputScan();

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
		if (Value > 0.75f)
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
	// Set variables for scanning
	HeadRelative = HeadsetCamera->GetComponentTransform().InverseTransformPosition(GetActorLocation());
	LeftRelative = LeftController->GetComponentTransform().InverseTransformPosition(HeadsetCamera->GetComponentLocation());
	RightRelative = RightController->GetComponentTransform().InverseTransformPosition(HeadsetCamera->GetComponentLocation());

	HeadRelVel = HeadLastRelPos - HeadRelative;
	LeftRelVel = -(LeftLastRelPos - LeftRelative);
	RightRelVel = -(RightLastRelPos - RightRelative);

	// Damage character for abruput velocity changes
	if ((VelocityLastTick - GetVelocity()).Size() > VelocityChangeDamageSpeed)
	{
		ApplyImpactDamage();
	}

	// Check for jump
	if (HeadRelVel.Z > JumpHeadReqZ && LeftRelVel.Z > JumpHandReqZ && RightRelVel.Z > JumpHandReqZ)
	{
		Jump();
	}

	// Check for sprint
	if (LeftRelVel.X + LeftRelVel.Z > SprintArmSwingReq && RightRelVel.X + RightRelVel.Z < SprintArmSwingReq * -0.1f && bHasForwardMovementInput ||
		RightRelVel.X + RightRelVel.Z > SprintArmSwingReq && LeftRelVel.X + LeftRelVel.Z < SprintArmSwingReq * -0.1f && bHasForwardMovementInput)
	{
		MotionSprint();
	}
	if (!GetCharacterMovement()->IsFalling() && GetCharacterMovement()->Velocity.Size() > MaxSprintSpeed * 0.9f)
	{
		LaunchCharacter(FVector(0.f, 0.f, 150.f), false, false);
	}


	// Debug Logging:
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("_"));
		GEngine->AddOnScreenDebugMessage(51, 0.f, FColor::Red, FString::Printf(TEXT("________________________________________PlayerWalkSpeed: %f"), GetCharacterMovement()->MaxWalkSpeed));
		GEngine->AddOnScreenDebugMessage(52, 0.f, FColor::Red, FString::Printf(TEXT("________________________________________PlayerVelocitySize: %f"), GetCharacterMovement()->Velocity.Size()));
		GEngine->AddOnScreenDebugMessage(53, 0.f, FColor::Red, FString::Printf(TEXT("________________________________________R: %s"), *RightRelVel.ToString()));
		GEngine->AddOnScreenDebugMessage(54, 0.f, FColor::Red, FString::Printf(TEXT("________________________________________L: %s"), *LeftRelVel.ToString()));
		GEngine->AddOnScreenDebugMessage(55, 0.f, FColor::Red, FString::Printf(TEXT("________________________________________H: %s"), *HeadRelVel.ToString()));

	}

	// Set variables for reference next tick
	HeadLastRelPos = HeadsetCamera->GetComponentTransform().InverseTransformPosition(GetActorLocation());
	LeftLastRelPos = LeftController->GetComponentTransform().InverseTransformPosition(HeadsetCamera->GetComponentLocation());
	RightLastRelPos = RightController->GetComponentTransform().InverseTransformPosition(HeadsetCamera->GetComponentLocation());
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
void AvrPlayer::MotionSprint()
{
	float OneThird = (MaxSprintSpeed - BaseCharacterSpeed) / 3.f;
	float SprintingSpeed = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed + OneThird, BaseCharacterSpeed, MaxSprintSpeed);
	GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

	if (!GetCharacterMovement()->IsFalling())
	{
		FVector Impulse = GetForwardController()->GetForwardVector();
		Impulse.Z = 0.f;
		float OneThird = (MaxSprintSpeed - BaseCharacterSpeed) / 3.f;
		FVector ImpulseToApply = Impulse.GetSafeNormal() * OneThird;

		LaunchCharacter(ImpulseToApply, false, false);
		
	}

	GetWorldTimerManager().SetTimer(SprintSpeedReturn_Handle, this, &AvrPlayer::ResetMaxWalkSpeed, SprintReturnTime, false);
}
void AvrPlayer::ResetMaxWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseCharacterSpeed;
}
UMotionControllerComponent * AvrPlayer::GetForwardController()
{
	if (LeftRelative.X > RightRelative.X)
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
	UGameplayStatics::OpenLevel(GetWorld(), "TestingMap");

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
