// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/LevelPool.h"

ELevelType ULevelPool::GetRandomLevelType()
{
	if (Levels.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rooms in pool"));
		return ELevelType::Standard;
	}

	float TotalWeight = 0.f;
	for (auto& [Room, RoomWeight] : Levels)
	{
		TotalWeight += RoomWeight;
	}

	const float RandomValue = FMath::RandRange(0.f, TotalWeight);
	float WeightSum = 0.f;

	for (auto& [Room, RoomWeight] : Levels)
	{
		WeightSum += RoomWeight;
		if (RandomValue <= WeightSum)
		{
			return Room;
		}
	}

	return ELevelType::Standard;
}
