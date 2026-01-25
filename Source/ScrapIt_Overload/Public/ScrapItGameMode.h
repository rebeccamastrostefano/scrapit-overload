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
	
	UPROPERTY(EditAnywhere, Category = "Config")
	class UWeaponsDatabase* WeaponsDatabase;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<AActor> BasicScrapBP;
	
protected:
	void BeginPlay() override;
	
};
