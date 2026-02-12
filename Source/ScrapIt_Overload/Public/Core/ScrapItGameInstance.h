// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/EnemyPool.h"
#include "Engine/GameInstance.h"
#include "Rooms/RoomPool.h"
#include "Scraps/ScrapBase.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponLevels.h"
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
	TMap<EScrapType, TSubclassOf<AScrapBase>> ScrapTypeToBP;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<EScrapType, UWeaponLevels*> WeaponLevels;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<EScrapType, TSubclassOf<AWeaponBase>> ScrapTypeToWeaponBP;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<int32, UEnemyPool*> RankToEnemyPool;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	URoomPool* RoomPool;
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<ERoomType, TSoftObjectPtr<UWorld>> RoomLevels;
	
	UFUNCTION()
	UEnemyPool* GetEnemyPool(const int32 Rank) const
	{
		return RankToEnemyPool.Contains(Rank) ? RankToEnemyPool[Rank] : nullptr;
	}
	
	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> DoorBP;
};
