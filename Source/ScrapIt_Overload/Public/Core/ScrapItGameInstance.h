// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rooms/EnemyPool.h"
#include "Engine/StreamableManager.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
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
	TSoftObjectPtr<UWorld> StartingLevel;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<UUserWidget> LoadingScreenWidget;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<EScrapType, TSubclassOf<AScrapBase>> ScrapTypeToBP;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<EScrapType, UWeaponLevels*> WeaponLevels;

	UPROPERTY(EditAnywhere, Category = "Global References")
	TMap<EScrapType, TSubclassOf<AWeaponBase>> ScrapTypeToWeaponBP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Global References")
	TMap<EScrapType, UTexture2D*> WeaponToImage;

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

	UPROPERTY(EditAnywhere, Category = "Global References")
	TSubclassOf<AActor> DoorBP;

	UPROPERTY()
	UUserWidget* LoadingScreen;

	TSoftObjectPtr<UWorld> PendingLevelLoading;
	TSharedPtr<FStreamableHandle> StreamingHandle;

	UFUNCTION()
	UEnemyPool* GetEnemyPool(const int32 Rank) const
	{
		return RankToEnemyPool.Contains(Rank) ? RankToEnemyPool[Rank] : nullptr;
	}

	UFUNCTION()
	void LoadStartingLevel() const
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, StartingLevel);
	}

	UFUNCTION()
	void LoadLevel(TSoftObjectPtr<UWorld> LevelToLoad);

	UFUNCTION()
	void OnLevelLoaded();
};
