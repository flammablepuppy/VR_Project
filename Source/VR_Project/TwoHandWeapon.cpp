// Copyright Aviator Games 2019


#include "TwoHandWeapon.h"
#include "Components/SphereComponent.h"

ATwoHandWeapon::ATwoHandWeapon()
{
	AftGripSphere = CreateDefaultSubobject<USphereComponent>("Aft Sphere");
	AftGripSphere->SetupAttachment(RootComponent);

	ForeGripSphere = CreateDefaultSubobject<USphereComponent>("Fore Sphere");
	ForeGripSphere->SetupAttachment(RootComponent);

}
void ATwoHandWeapon::BeginPlay()
{
	Super::BeginPlay();

}
void ATwoHandWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/**
*
*/

