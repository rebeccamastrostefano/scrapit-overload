// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScrapBase.h"
#include "ScrapLootTable.generated.h"


USTRUCT(BlueprintType)
struct FScrapDrop
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AScrapBase> ScrapClass;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.1f;
};

UCLASS()
class SCRAPIT_OVERLOAD_API UScrapLootTable : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Loot")
	TArray<FScrapDrop> PossibleDrops;
	
	UPROPERTY(EditAnywhere, Category = "Loot")
	int32 MinNumDrops = 0;
	
	UPROPERTY(EditAnywhere, Category = "Loot")
	int32 MaxNumDrops = 2;
};
