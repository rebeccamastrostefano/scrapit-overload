// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/LevelPool.h"

ELevelType ULevelPool::GetRandomLevelType()
{
	if (Levels.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Levels in pool"));
		return ELevelType::Standard;
	}

	float TotalWeight = 0.f;
	for (auto& [Level, LevelWeight, Map] : Levels)
	{
		TotalWeight += LevelWeight;
	}

	const float RandomValue = FMath::RandRange(0.f, TotalWeight);
	float WeightSum = 0.f;

	for (auto& [Level, LevelWeight, Map] : Levels)
	{
		WeightSum += LevelWeight;
		if (RandomValue <= WeightSum)
		{
			return Level;
		}
	}

	return ELevelType::Standard;
}

TSoftObjectPtr<UWorld> ULevelPool::GetLevelMap(ELevelType LevelType)
{
	if (LevelType == ELevelType::Standard)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelPool: No Map for Standard Level"));
		return nullptr;
	}

	FLevelWeight* LevelWeight = Levels.FindByPredicate([LevelType](const FLevelWeight& Level)
	{
		return Level.Level == LevelType;
	});

	return LevelWeight->Map;
}
