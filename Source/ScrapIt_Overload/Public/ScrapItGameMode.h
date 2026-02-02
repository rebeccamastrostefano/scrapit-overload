// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ScrapItGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API AScrapItGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AScrapItGameMode();
	
protected:
	void BeginPlay() override;
	
};
