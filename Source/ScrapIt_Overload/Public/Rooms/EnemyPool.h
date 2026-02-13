// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Engine/DataAsset.h"
#include "EnemyPool.generated.h"

USTRUCT()
struct FEnemyDetails
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemyBase> EnemyClass;
	
	UPROPERTY(EditAnywhere)
	int32 ClusterCount = 1;
	
	UPROPERTY(EditAnywhere)
	float SpawnChance = 1.f;
};
UCLASS()
class SCRAPIT_OVERLOAD_API UEnemyPool : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<FEnemyDetails> Enemies;
	
	FEnemyDetails GetRandomEnemyBasedOnChance();
};
