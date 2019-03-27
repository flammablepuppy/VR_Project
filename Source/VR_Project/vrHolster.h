// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "vrHolster.generated.h"

UCLASS()
class VR_PROJECT_API AvrHolster : public AActor
{
	GENERATED_BODY()
	
public:	
	AvrHolster();
protected:
	virtual void BeginPlay() override;
public:	
	//virtual void Tick(float DeltaTime) override;

protected:

	//		COMPONENTS
	//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* HolsterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USphereComponent* HolsterSphere;

	//		VARIABLES
	//

	UPROPERTY(BlueprintReadOnly)
	class AvrPickup* HolsteredItem;

	//		FUNCTIONS
	//

	UFUNCTION()
	void SubscribeCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void UnsubCatch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void CatchDroppedPickup(AvrPickup* DroppedPickup);

	UFUNCTION()
	void EnableHolsteredItem(AvrPickup* PickupToEnable);

	UFUNCTION()
	void ClearHolsteredItem(AvrPickup* DroppedPickup);
};
