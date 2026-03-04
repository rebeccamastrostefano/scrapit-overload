// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelPool.generated.h"

UENUM(BlueprintType)
enum class ELevelType : uint8
{
	Standard,
	Shop,
	Maintenance,
	Treasure,
	Gamble,
	Random
};

USTRUCT()
struct FLevelWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ELevelType Level;

	UPROPERTY(EditAnywhere)
	float Weight;
};

UCLASS()
class SCRAPIT_OVERLOAD_API ULevelPool : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FLevelWeight> Levels;

	ELevelType GetRandomLevelType();
};
