// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScrapActor.h"
#include "WeaponsDatabase.generated.h"

/**
 * 
 */
UCLASS()
class SCRAPIT_OVERLOAD_API UWeaponsDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TMap<EScrapType, TSubclassOf<AActor>> WeaponLibrary;
};
