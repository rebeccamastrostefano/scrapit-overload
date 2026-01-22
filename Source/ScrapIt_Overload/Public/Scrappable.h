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
	virtual void OnMagnetPulled(FVector MagnetLocation, float PullStrength) = 0;
	
	float Weight = 1.0f;
};
