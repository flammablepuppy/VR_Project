// Copyright Aviator Games 2019

#pragma once

#include "CoreMinimal.h"
#include "SpecialVariables.h"
#include "Components/ActorComponent.h"
#include "StatsHandler.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VR_PROJECT_API UStatsHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatsHandler();

protected:
	virtual void BeginPlay() override;

};
