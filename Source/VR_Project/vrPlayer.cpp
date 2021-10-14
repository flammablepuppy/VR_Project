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
#include "Sound/SoundCue.h"
#include "RaceGameMode.h"

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
	LeftController->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	LeftController->MotionSource = "Left";

	LeftVolume = CreateDefaultSubobject<USphereComponent>("Left Pickup Scan Volume");
	LeftVolume->SetupAttachment(LeftController);

	RightController = CreateDefaultSubobject<UMotionControllerComponent>("Right Controller");
	RightController->SetupAttachment(vrRoot);
	RightController->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
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

	PlayerInputComponent->BindAction("LeftStickClick", IE_Pressed, this, &AvrPlayer::SmallJump);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AvrPlayer::MenuPressed);
	PlayerInputComponent->BindAction("Menu", IE_Released, this, &AvrPlayer::MenuReleased);
}
void AvrPlayer::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = SprintMinSpeed;

	/** Initialize for use with MotionInput */
	HeadLastRelPos = HeadsetCamera->GetComponentLocation();
	LeftLastRelPos = LeftController->GetComponentLocation();
	RightLastRelPos = RightController->GetComponentLocation();

	/** Prevent MotionImput from firing anything for a moment after starting */
	GetWorldTimerManager().SetTimer(SprintLeft_Timer, 0.5f, false);
	GetWorldTimerManager().SetTimer(SprintRight_Timer, 0.5f, false);
	GetWorldTimerManager().SetTimer(SpawnJumpPrevention_Timer, 0.5f, false);

}
void AvrPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OffsetRoot();

	// Tick MotionInputScan while alive
	if (!HealthStatsComp->GetIsDead()) { MotionInputScan(); }

	// Interpolates back to a normal friction value after sprinting has ended
	if (GetCharacterMovement()->MaxWalkSpeed == SprintMinSpeed && GetCharacterMovement()->BrakingFriction != SprintNormalFriction)
	{
		WhatsTheFriction = GetCharacterMovement()->BrakingFriction;
		float NewFriction = FMath::Clamp(GetCharacterMovement()->BrakingFriction + (GetWorld()->GetDeltaSeconds() * 2.f), SprintingFriction, SprintNormalFriction);
		GetCharacterMovement()->BrakingFriction = NewFriction;
		//UE_LOG(LogTemp, Warning, TEXT("Friction adjusting on Tick: %f"), NewFriction)
	}

	//UE_LOG(LogTemp, Warning, TEXT("Head: %s | Left: %s | Right: %s"), *HeadRelative.ToString(), *LeftRelative.ToString(), *RightRelative.ToString())

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
void AvrPlayer::SmallJump()
{
	GetCharacterMovement()->JumpZVelocity = SmallJumpHeight;
	Jump();
	if (!GetCharacterMovement()->IsFalling()) UGameplayStatics::PlaySound2D(this, SmallJumpSound);
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
	FVector MeasuringPoint = GetCapsuleComponent()->GetComponentLocation(); 
	MeasuringPoint.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	HeadRelative = MeasuringPoint - HeadsetCamera->GetComponentLocation(); // Head is always X and Y 0, so no need to inverse transform
	LeftRelative = LeftController->GetComponentTransform().InverseTransformPosition(MeasuringPoint);
	RightRelative = RightController->GetComponentTransform().InverseTransformPosition(MeasuringPoint);

	//	With those relative positions, measure each components relative velocity based off of their positions last tick (set at the end of this function)
	HeadRelVel = HeadLastRelPos - HeadRelative;
	LeftRelVel = LeftLastRelPos - LeftRelative;
	RightRelVel = RightLastRelPos - RightRelative;

		/** IMPACT DAMAGE
		*	Damage character for abruput velocity changes
		*/
		if ((VelocityLastTick - GetVelocity()).Size() > VelocityChangeDamageSpeed && 
			!GetHealthStats()->GetIsDead() && 
			bImpactDamageActive)
		{
			ApplyImpactDamage();
		}
		
		/** MOTION JUMP
		*	Trigger jump when conditions are met
		*	High jump with forward impulse when arms swing and head pops
		*	Small jump when arms are whipped up quickly while moving forward
		*/
		if (LeftRelVel.Z > BigJumpHandReq && 
			RightRelVel.Z > BigJumpHandReq  && 
			HeadRelVel.Z > BigJumpHeadReq &&
			!GetWorldTimerManager().IsTimerActive(SpawnJumpPrevention_Timer) &&
			!GetCharacterMovement()->IsFalling() &&
			!GetWorldTimerManager().IsTimerActive(Leap_Timer) ||

			LeftRelVel.Z > BigJumpHandReq &&
			RightRelVel.Z > BigJumpHandReq &&
			!GetCharacterMovement()->IsFalling() &&
			!GetWorldTimerManager().IsTimerActive(Leap_Timer) &&
			GetCharacterMovement()->Velocity.Size() > SprintMaxSpeed * BigJumpHighSpeedFireMultiplier)
		{
			// Set timer for cooldown, prevents multiple fires when jump up an incline
			GetWorldTimerManager().SetTimer(Leap_Timer, 0.4f, false);

			// Prevent sprint from firing simultaneously with the jump
			GetWorldTimerManager().SetTimer(SprintLeft_Timer, SprintCooldownDuration, false);
			GetWorldTimerManager().SetTimer(SprintRight_Timer, SprintCooldownDuration, false);

			// Fire
			MotionJump();

		}

		/** MOTION SPRINT
		*	Add an impulse in the direction the forward arm swings
		*	Triggered when the forward swinging arm x and y relative velocities combined exceed a threshold while the other hand is going in reverse by half that speed
		*/
		float LeftSwing = LeftRelVel.X + LeftRelVel.Z;
		float RightSwing = RightRelVel.X + RightRelVel.Z;
		FTimerDelegate SprintReset_Delegate;
		SprintReset_Delegate.BindUFunction(GetHealthStats(), "SetSpeed", SprintMinSpeed, -1.f);

		// Make movement input a requirement for impulses, they fire on accident too often otherwise
		if (GetCharacterMovement()->Velocity.Size() > 50.f)
		{
			// LEFT HAND
			if (LeftSwing > SprintMinImpulseSpeed &&
				RightSwing < -SprintMinImpulseSpeed * 0.45f &&
				!GetWorldTimerManager().IsTimerActive(SprintLeft_Timer) &&
				!GetCharacterMovement()->IsFalling() &&
				!GetHealthStats()->EffectIsActive(EEffectTag::Tag_Slow))
			{
				// Set timer for cooldown
				GetWorldTimerManager().SetTimer(SprintLeft_Timer, SprintCooldownDuration, false);

				// Set timer for sprint end
				GetWorldTimerManager().SetTimer(SprintReset_Timer, SprintReset_Delegate, SprintDecelResetDuration, false);

				// Determine impulse power
				float Percent = LeftSwing - SprintMinImpulseSpeed / SprintMaxImpulseSpeed;
				if (Percent > 1.f) Percent = 1.f;

				// Find proper direction to apply impulse
				// Method compensates for the way the hand points the controller while swinging arms
				// Each hand requires a different rotation
				FVector Direction = (LeftController->GetComponentLocation() - SprintLeftLastPos).GetSafeNormal2D();
				FRotator DirRot = Direction.ToOrientationRotator();
				DirRot -= FRotator(0.f, 10.f, 0.f);
				Direction = DirRot.Vector();
				Direction.GetSafeNormal();

				// Fire
				MotionSprint(Percent, Direction);
			}

			// RIGHT HAND
			if (RightSwing > SprintMinImpulseSpeed &&
				LeftSwing < -SprintMinImpulseSpeed * 0.45f &&
				!GetWorldTimerManager().IsTimerActive(SprintRight_Timer) &&
				!GetCharacterMovement()->IsFalling() &&
				!GetHealthStats()->EffectIsActive(EEffectTag::Tag_Slow))
			{
				// Set timer for cooldown
				GetWorldTimerManager().SetTimer(SprintRight_Timer, SprintCooldownDuration, false);

				// Set timer for sprint end
				GetWorldTimerManager().SetTimer(SprintReset_Timer, SprintReset_Delegate, SprintDecelResetDuration, false);

				// Determine impulse power
				float Percent = RightSwing - SprintMinImpulseSpeed / SprintMaxImpulseSpeed;
				if (Percent > 1.f) Percent = 1.f;

				// Find proper direction to apply impulse
				// Method compensates for the way the hand points the controller while swinging arms
				// Each hand requires a different rotation
				FVector Direction = (RightController->GetComponentLocation() - SprintRightLastPos).GetSafeNormal2D();
				FRotator DirRot = Direction.ToOrientationRotator();
				DirRot += FRotator(0.f, 10.f, 0.f);
				Direction = DirRot.Vector();
				Direction.GetSafeNormal();

				// Fire
				MotionSprint(Percent, Direction);
			}

			// SPRINT TIMER CLEANUP
			// Falling faster than walking and speed isn't already pegged, max speed out, not slowed
			if (GetCharacterMovement()->IsFalling() &&
				GetCharacterMovement()->Velocity.Size() > SprintMinSpeed &&
				GetCharacterMovement()->MaxWalkSpeed < SprintMaxSpeed &&
				!GetHealthStats()->EffectIsActive(EEffectTag::Tag_Slow))
			{
				GetHealthStats()->ApplyHaste(SprintMaxSpeed, -1.f, true, SprintMaxSpeed);
				GetWorldTimerManager().SetTimer(SprintReset_Timer, SprintReset_Delegate, 0.1f, false);
				GetWorldTimerManager().PauseTimer(SprintReset_Timer);

				// Lower ground friction to simulate higher momentum
				GetCharacterMovement()->GroundFriction = SprintingFriction;

			}

			if (!GetCharacterMovement()->IsFalling() &&
				GetWorldTimerManager().IsTimerPaused(SprintReset_Timer))
			{
				GetWorldTimerManager().UnPauseTimer(SprintReset_Timer);
			}
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

	UGameplayStatics::ApplyDamage(this, AppliedDamage, this->GetController(), this, MotionDamage);
	UGameplayStatics::PlaySound2D(this, ImpactDamageSound);

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

	}

}
void AvrPlayer::MotionJump()
{
	if (!GetCharacterMovement()->IsFalling()) UGameplayStatics::PlaySoundAtLocation(GetWorld(), BigJumpSound, GetActorLocation());

	// Prevent sprint impulses from amplifying your jump distance
	if (GetCharacterMovement()->Velocity.Size() > SprintMaxSpeed && 
		GetCharacterMovement()->Velocity.Size() < SprintMaxSpeed * BigJumpHighSpeedFireMultiplier)
	{
		GetCharacterMovement()->Velocity *= (SprintMaxSpeed / GetCharacterMovement()->Velocity.Size());
	}

	// Set jump power and fire
	GetCharacterMovement()->JumpZVelocity = BigJumpHeight;
	Jump();

	// Add impulse to aid in clearing gaps
	if (GetCharacterMovement()->Velocity.Size() > 0.f)
	{
		// Calculate impulse
		FVector VelocityDirection = GetCharacterMovement()->Velocity.GetSafeNormal2D();
		FVector LookDirection = HeadsetCamera->GetForwardVector().GetSafeNormal2D();
		FVector ImpulseDirection = (VelocityDirection + LookDirection).GetSafeNormal2D();
		FVector Impulse = ImpulseDirection * PostJumpImpulse;

		// Calculate chained jump impulse: this is multiplicative so the faster you're going the bigger boost you get
		if (GetCharacterMovement()->Velocity.Size() > SprintMaxSpeed * BigJumpHighSpeedFireMultiplier)
		{
			float ChainBonus = FMath::Clamp(GetCharacterMovement()->Velocity.Size() * ChainJumpMultiplier, 0.f, VelocityChangeDamageSpeed);
			FVector ChainBonusImpulse = Impulse + (ImpulseDirection * ChainBonus);
			GetCharacterMovement()->Velocity += ChainBonusImpulse;
		}
		// Regular jump impulse
		else GetCharacterMovement()->Velocity += Impulse;

		// Fire
		GetCharacterMovement()->UpdateComponentVelocity();
	}

}
void AvrPlayer::MotionSprint(float ImpulsePercent, FVector Direction)
{
	// Lower ground friction to simulate higher momentum
	GetCharacterMovement()->GroundFriction = SprintingFriction;

	// Increase walk speed, timer in MotionScan will return walk speed
	float SpeedIncrement = (SprintMaxSpeed - SprintMinSpeed) / 3.f;
	GetHealthStats()->ApplyHaste(SpeedIncrement, -1.f, true, SprintMaxSpeed);

	// Apply an impulse
	if (!GetCharacterMovement()->IsFalling())
	{
		// Add impulse to velocity
		GetCharacterMovement()->Velocity += (Direction * ImpulsePercent * SpeedIncrement);

		// Prevent impulse from making you move faster than SprintMaxSpeed
		if (GetCharacterMovement()->Velocity.Size() > SprintMaxSpeed)
		{
			GetCharacterMovement()->Velocity *= ((SprintMaxSpeed * BigJumpHighSpeedFireMultiplier) / GetCharacterMovement()->Velocity.Size());
		}

		// Fire
		GetCharacterMovement()->UpdateComponentVelocity();

	}
}

// Interaction Calls
void AvrPlayer::LeftGripPull()
{
	if (LeftHeldObject)
	{
		if (!LeftHeldObject->GetCanDrop()) return;
	}

	ScanForClosestObject(LeftVolume, LeftScanTarget, LeftController);
	ExecuteGrip(LeftScanTarget, LeftHeldObject, LeftController);
	if (OnGrip.IsBound())
	{
		OnGrip.Broadcast(LeftController);
		OnGrip.Clear();
	}
}
void AvrPlayer::LeftGripRelease()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController && LeftHeldObject->GetCanDrop())
	{
		ExecuteDrop(LeftHeldObject);
	}
	else if(GetWorldTimerManager().IsTimerActive(ForceLeftDrop_Timer))
	{
		ForceDropLeft();
	}
	else
	{
		GetWorldTimerManager().SetTimer(ForceLeftDrop_Timer, ForceDropTime, false);
	}
}
void AvrPlayer::LeftTriggerHandle(float Value)
{
	if (Value > 0.f && LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TriggerPulled(Value);
	}
}
void AvrPlayer::LeftTopPush()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TopPushed();
	}
}
void AvrPlayer::LeftTopRelease()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->TopReleased();
	}
}
void AvrPlayer::LeftBottomPush()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->BottomPushed();
	}
}
void AvrPlayer::LeftBottomRelease()
{
	if (LeftHeldObject && LeftHeldObject->GetOwningMC() == LeftController)
	{
		LeftHeldObject->BottomReleased();
	}
}

void AvrPlayer::RightGripPull()
{
	if (RightHeldObject) 
	{
		if (!RightHeldObject->GetCanDrop()) return;
	}

	ScanForClosestObject(RightVolume, RightScanTarget, RightController);
	ExecuteGrip(RightScanTarget, RightHeldObject, RightController);
	if (OnGrip.IsBound())
	{
		OnGrip.Broadcast(RightController);
		OnGrip.Clear();
	}
}
void AvrPlayer::RightGripRelease()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController && RightHeldObject->GetCanDrop())
	{
		ExecuteDrop(RightHeldObject);
	}
	else if(GetWorldTimerManager().IsTimerActive(ForceRightDrop_Timer))
	{
		ForceDropRight();
	}
	else
	{
		GetWorldTimerManager().SetTimer(ForceRightDrop_Timer, ForceDropTime, false);
	}
}
void AvrPlayer::RightTriggerHandle(float Value)
{
	if (Value > 0.f && RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TriggerPulled(Value);
	}
}
void AvrPlayer::RightTopPush()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TopPushed();
	}
}
void AvrPlayer::RightTopRelease()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->TopReleased();
	}
}
void AvrPlayer::RightBottomPush()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->BottomPushed();
	}
}
void AvrPlayer::RightBottomRelease()
{
	if (RightHeldObject && RightHeldObject->GetOwningMC() == RightController)
	{
		RightHeldObject->BottomReleased();
	}
}

void AvrPlayer::MenuPressed()
{
	if (GetWorldTimerManager().IsTimerActive(MenuPress_Timer)) return;
	else
	{
		MenuRequested();
		// FTimerDelegate Del;
		// Del.BindUFunction(this, FName("MenuRequested"));
		// GetWorldTimerManager().SetTimer(MenuPress_Timer, Del, MenuButtonHoldTime, false);
	}
} 

void AvrPlayer::MenuReleased()
{
	if (GetWorldTimerManager().IsTimerActive(MenuPress_Timer))
		GetWorldTimerManager().ClearTimer(MenuPress_Timer);
}

void AvrPlayer::ResetTestingMap()
{
	if (bCommitsSeppuku)
	{
		UGameplayStatics::ApplyDamage(this, 500.f, this->GetController(), this, MotionDamage);
	}
	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelToLoad);

	}
}
void AvrPlayer::AssignLeftGrip(AvrPickup* NewGrippedObject)
{
	if (LeftHeldObject) LeftHeldObject = nullptr;

	ExecuteGrip(NewGrippedObject, LeftHeldObject, LeftController);
	if (OnGrip.IsBound())
	{
		OnGrip.Broadcast(LeftController);
		OnGrip.Clear();
	}
}
void AvrPlayer::AssignRightGrip(AvrPickup* NewGrippedObject)
{
	if (RightHeldObject) RightHeldObject = nullptr;

	ExecuteGrip(NewGrippedObject, RightHeldObject, RightController);
	if (OnGrip.IsBound())
	{
		OnGrip.Broadcast(RightController);
		OnGrip.Clear();
	}
}
void AvrPlayer::ForceDropLeft()
{
	if (LeftHeldObject && !LeftHeldObject->GetCanDrop())
	{
		LeftHeldObject->SetCanDrop(true);
		LeftGripRelease();
	}
}
void AvrPlayer::ForceDropRight()
{
	if (RightHeldObject && !RightHeldObject->GetCanDrop())
	{
		RightHeldObject->SetCanDrop(true);
		RightGripRelease();
	}
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
	if (ObjectToDrop && ObjectToDrop->GetCanDrop())
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

void AvrPlayer::SetImpactDamageEnabled(bool NewState)
{
	bImpactDamageActive = NewState;
}
