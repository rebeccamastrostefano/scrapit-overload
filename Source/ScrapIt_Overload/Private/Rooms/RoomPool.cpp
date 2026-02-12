// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/RoomPool.h"

ERoomType URoomPool::GetRandomRoomType()
{
	if (Rooms.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rooms in pool"));
		return ERoomType::Standard;
	}

	float TotalWeight = 0.f;
	for (auto& [Room, RoomWeight] : Rooms)
	{
		TotalWeight += RoomWeight;
	}
	
	const float RandomValue = FMath::RandRange(0.f, TotalWeight);
	float WeightSum = 0.f;
	
	for (auto& [Room, RoomWeight] : Rooms)
	{
		WeightSum += RoomWeight;
		if (RandomValue <= WeightSum)
		{
			return Room;
		}
	}
	
	return ERoomType::Standard;
}