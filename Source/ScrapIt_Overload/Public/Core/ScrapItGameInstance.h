// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/EnemyPool.h"
#include "Engine/GameInstance.h"
#include "ScrapItGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UScrapItGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> BasicScrapBP;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> GoldenScrapBP;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<int32, UEnemyPool*> RankToEnemyPool;
	
	UFUNCTION()
	UEnemyPool* GetEnemyPool(const int32 Rank)
	{
		return RankToEnemyPool.Contains(Rank) ? RankToEnemyPool[Rank] : nullptr;
	}
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> DoorBP;
};
