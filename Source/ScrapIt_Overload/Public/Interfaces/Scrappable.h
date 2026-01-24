// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scrappable.generated.h"

UINTERFACE()
class UScrappable : public UInterface
{
	GENERATED_BODY()
};

class IScrappable
{
	GENERATED_BODY()

public:
	virtual void OnMagnetPulled(AActor* MechaActor, float PullStrength, float PullRadius, float CollectionRadius) = 0;
	virtual void OnMagnetReleased() = 0;
	virtual void OnCollected() = 0;
	
	float BasePullSpeed = 1.0f;
	float RiseHeight = 150.0f;
};
