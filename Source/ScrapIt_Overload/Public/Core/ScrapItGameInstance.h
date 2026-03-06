// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/EnemyPool.h"
#include "Engine/GameInstance.h"
#include "Rooms/LevelPool.h"
#include "Rooms/Objectives/RoomObjective.h"
#include "Rooms/RoomsPool.h"
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
	ULevelPool* LevelPool;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<ELevelType, TSoftObjectPtr<UWorld>> RoomLevels;

	UPROPERTY(EditAnywhere, Category = "Global References")
	URoomsPool* RoomsPool;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<ERoomType, TSubclassOf<URoomObjective>> RoomObjectives;

	UFUNCTION()
	UEnemyPool* GetEnemyPool(const int32 Rank) const
	{
		return RankToEnemyPool.Contains(Rank) ? RankToEnemyPool[Rank] : nullptr;
	}

	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> DoorBP;
};
