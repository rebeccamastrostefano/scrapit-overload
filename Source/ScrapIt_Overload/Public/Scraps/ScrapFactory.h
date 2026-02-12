// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScrapBase.h"
#include "ScrapLootTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ScrapFactory.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UScrapFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void SpawnScraps(const UObject* WorldContextObject, const FVector& Location, const UScrapLootTable* LootTable, const int32 BaseDropAmount);
	
protected:
	static void FinalizeSpawn(const AScrapBase* NewScrap);
};
