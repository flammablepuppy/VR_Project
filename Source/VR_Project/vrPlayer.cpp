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
	GetCharacterMovement()->BrakingDecelerationWalking = BaseCharacterSpeed * 2.f;

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

	ApplyImpactDamage((VelocityLastTick - GetVelocity()).Size());
	MotionJump();
	MotionSprint();

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
void AvrPlayer::ApplyImpactDamage(float VelocityChange)
{
	if (VelocityChange > VelocityChangeDamageSpeed)
	{
		float AppliedDamage = VelocityChange - VelocityChangeDamageSpeed;
		AppliedDamage *= ExponentialImpactDamage;
		AppliedDamage *= AppliedDamage; // Having the damage be exponential makes it feel much more fair. Big falls hurt a lot, little ones not so much
		if (AppliedDamage < MinimumImpactDamage)
		{
			AppliedDamage = MinimumImpactDamage;
		}

		if (AppliedDamage > 65.f) // Hard impacts cause player to drop anything they're holding
		{
			if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
			{
				ExecuteDrop(LeftHeldObject);
			}
			if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
			{
				ExecuteDrop(RightHeldObject);
			}
		}

		UGameplayStatics::ApplyDamage(this, AppliedDamage, this->GetController(), this, MotionDamage);
	}
}
void AvrPlayer::MotionJump()
{
	if (HeadRelVel.Z > JumpHeadReqZ && LeftRelVel.Z > JumpHandReqZ && RightRelVel.Z > JumpHandReqZ)
	{
		Jump();

		// Give a slight forward boost to forward movement when jumping
		if (bHasForwardMovementInput)
		{
			float BoostAmount = FMath::Clamp(GetVelocity().Size() * 1.2f, 0.f, MaxSprintSpeed * 1.5f);
			FVector ForwardBoost = HeadsetCamera->GetForwardVector().GetSafeNormal2D() * BoostAmount;
			LaunchCharacter(ForwardBoost, false, false);
		}
	}
}
void AvrPlayer::MotionSprint()
{
	// Determine forward and rear controllers 
	UMotionControllerComponent* ForwardController;
	FVector ForwardControllerVel;
	FVector RearControllerVel;
	if (LeftRelative.X > RightRelative.X)
	{
		ForwardController = LeftController;
		ForwardControllerVel = LeftRelVel;
		RearControllerVel = RightRelVel;
	}
	else
	{
		ForwardController = RightController;
		ForwardControllerVel = RightRelVel;
		RearControllerVel = LeftRelVel;
	}

	// Check if they're swinging their arms properly, increase speed
	if (ForwardControllerVel.X + ForwardControllerVel.Z > SprintArmSwingReq &&
		RearControllerVel.X + RearControllerVel.Z < -0.1f &&
		bHasForwardMovementInput)
	{
		// Increase speed, adjust friction and decel
		float SprintBonus = FMath::Clamp(GetCharacterMovement()->MaxWalkSpeed + ForwardControllerVel.Size() * 15.f, BaseCharacterSpeed, MaxSprintSpeed);
		GetCharacterMovement()->MaxWalkSpeed = SprintBonus;
		GetCharacterMovement()->BrakingDecelerationWalking = 0.f;
		GetCharacterMovement()->GroundFriction = 4.f;

		GetWorldTimerManager().SetTimer(SprintBoundCharged_Timer, SprintBoundChargedDuration, false);
		GetWorldTimerManager().SetTimer(SprintSpeedReturn_Handle, this, &AvrPlayer::AdjustMaxWalkSpeed, SprintReturnTime, false);
	}

	// Take a bounding leap if not falling or crouching
	if (GetWorldTimerManager().IsTimerActive(SprintBoundCharged_Timer) &&!GetCharacterMovement()->IsFalling() && !GetCharacterMovement()->IsCrouching())
	{
		GetWorldTimerManager().SetTimer(SprintBoundCharged_Timer, 0.f, false);

		FVector SprintImpulse = ForwardController->GetForwardVector();
		SprintImpulse.Z = 0.f;
		FRotator SprintForwardRot = SprintImpulse.Rotation();
		SprintForwardRot.Yaw += FMath::Clamp(HeadsetCamera->GetForwardVector().Rotation().Yaw - SprintForwardRot.Yaw, -30.f, 30.f);
		SprintImpulse = SprintForwardRot.Vector();

		if (GetCharacterMovement()->Velocity.Size()  + MaxSprintSpeed * 0.6f > MaxSprintSpeed * 1.1)
		{
			SprintImpulse.Z = SprintBoundHeight;
			LaunchCharacter(SprintImpulse, false, false);
			SprintImpulse *= GetMovementComponent()->Velocity.Size() * MaxSprintSpeed / GetMovementComponent()->Velocity.Size();
			SprintImpulse.Z = 0.f;
			GetMovementComponent()->Velocity = SprintImpulse * 1.1f;
			GetMovementComponent()->UpdateComponentVelocity();
		}
		else
		{
			SprintImpulse *= MaxSprintSpeed * 0.6f;
			SprintImpulse.Z = SprintBoundHeight;
			LaunchCharacter(SprintImpulse, false, false);
		}
	}
}
void AvrPlayer::AdjustMaxWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseCharacterSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = BaseCharacterSpeed;
	GetCharacterMovement()->GroundFriction = 0.5f;
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
