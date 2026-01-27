// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Scraps/ScrapActor.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UGameManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Transient)
	TSubclassOf<AScrapActor> BasicScrapBlueprint;
	
	void SpawnRandomScrapsAtLocation(FVector Location, int8 Amount);
};
